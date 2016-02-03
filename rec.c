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

int main(int argc, char **argv)
{
	struct input_event event;
	char cmd[64];
	int i;
	FILE *f;
	int fd_ev, fd_snd;
	int samples = 0;
	int triggered = 0;
	int r;
	FILE *fout = NULL;
	int head = 0, tail = 0;
	int16_t hist[HISTSIZE];
	int count[256] = { 0 };
	
	if(argc < 2) {
		fprintf(stderr, "usage: %s </dev/input/event#>\n", argv[0]);
		exit(1);
	}

	f = popen("arecord -D plughw:1,0 -f cd -r 44100 -c 1", "r");
	fd_snd = fileno(f);

	fd_ev = open(argv[1], O_RDONLY);
	if(fd_ev == -1) {
		perror("Could not open event input");
		exit(1);
	}

	fd_set fds;
	while(1) {
		FD_ZERO(&fds);
		FD_SET(fd_ev, &fds);
		FD_SET(fd_snd, &fds);

		r = select(16, &fds, NULL, NULL, NULL);

		if(FD_ISSET(fd_ev, &fds)) {

			read(fd_ev, &event, sizeof event);

			if(event.type != 1) continue;
			if(event.value == 2) continue;

			snprintf(cmd, sizeof cmd, "sox -qq -r 44100 -e signed -b 16 -c 1 -t raw - wav-new/%02x-%d.wav", event.code, event.value);
			fout = popen(cmd, "w");
			samples = 0;
			triggered = 1;
			printf("%02x %d ", event.code, event.value);
			fflush(stdout);
		}

		if(FD_ISSET(fd_snd, &fds)) {
			int16_t buf;
			r = read(fd_snd, &buf, sizeof buf);

			if(triggered) {
				if(samples == 0) {
					if(buf < -3000 || buf > 3000) {
						samples = 1;
						printf(">");
						fflush(stdout);
					}
				}

				if(fout && samples) {
					fwrite(&hist[head], 1, sizeof hist[head], fout);
					if(samples > 44000/3) {
						fclose(fout);
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

