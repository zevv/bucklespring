#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>


struct input_event {
	struct timeval time;
	unsigned short type;
	unsigned short code;
	unsigned int value;
};

#define HISTSIZE 60

void
samplename(char *buf, size_t buf_len, unsigned short code, unsigned int value)
{
	int i;

	for (i = 0; i < 10; i++)
	{
		snprintf(buf, buf_len, "wav-new/%02x-%d-%d.wav", code, value, i);
		if (access(buf, R_OK) == -1)
			return;
	}

	fprintf(stderr, "Damn, you recorded too many samples for %02x %d\n", code, value);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	struct input_event event;
	char cmd[256];
	char fname[256];
	FILE *f;
	int fd_snd;
	int samples = 0;
	int triggered = 0;
	FILE *fout = NULL;
	int head = 0;
	int16_t hist[HISTSIZE];

	/* Run like:
	 *
	 *     $ (sleep 2; sudo stdbuf -i0 -o0 cat /dev/input/event5) | ./rec
	 *
	 * Begin with a sleep, so we don't catch the "Return, released"
	 * event that started this command. */
	fprintf(stderr, "Waiting for evdev data on stdin ...\n");

	//f = popen("arecord -D plughw:1,0 -f cd -r 44100 -c 1", "r");
	f = popen("parec --rate=44100 --format=s16le --channels=1", "r");
	fd_snd = fileno(f);

	fd_set fds;
	while(1) {
		FD_ZERO(&fds);
		FD_SET(STDIN_FILENO, &fds);
		FD_SET(fd_snd, &fds);

		select(16, &fds, NULL, NULL, NULL);

		if(FD_ISSET(STDIN_FILENO, &fds)) {

			read(STDIN_FILENO, &event, sizeof event);

			if(event.type != 1) continue;
			if(event.value == 2) continue;

			if(triggered == 0) {
				samplename(fname, sizeof fname, event.code, event.value);
				snprintf(cmd, sizeof cmd, "sox -qq -r 44100 -e signed -b 16 -c 1 -t raw --norm=-3 - %s", fname);
				printf("%02x %d: ", event.code, event.value);
				fflush(stdout);
				fout = popen(cmd, "w");
				samples = 0;
				triggered = 1;
				printf("%02x %d ", event.code, event.value);
				fflush(stdout);
			}
		}

		if(FD_ISSET(fd_snd, &fds)) {
			int16_t buf;
			read(fd_snd, &buf, sizeof buf);

			if(triggered) {
				if(samples == 0) {
					if(buf < -1000 || buf > 1000) {
						samples = 1;
						printf(">");
						fflush(stdout);
					}
				}

				if(fout && samples) {
					fwrite(&hist[head], 1, sizeof hist[head], fout);
					if(samples > 44000/3) {
						fclose(fout);
						snprintf(cmd, sizeof(cmd), "paplay %s &", fname);
						system(cmd);
						printf(".\n");
						fout = NULL;
						triggered = 0;
					}
				}

				if(samples) samples ++;
			}

			hist[head] = buf;
			head = (head + 1) % HISTSIZE;
		}

	}

	return 0;
}

