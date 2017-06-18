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

/* We keep recording all the time, creating a history of BUF_HIST bytes.
 * Once a key action has been registered, we store our history and
 * record another BUF_POST bytes.
 *
 * Those exact values depend on your machine. We don't do
 * auto-detection. */
#define BUF_HIST (44100 / 32)
#define BUF_POST (44100 / 2)

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
	int triggered = 0;
	FILE *fout = NULL;
	int16_t hist[BUF_HIST], post[BUF_POST];
	size_t hist_at = 0;

	/* Run like:
	 *
	 *     $ (sleep 2; sudo stdbuf -i0 -o0 cat /dev/input/event5) | ./rec
	 *
	 * Begin with a sleep, so we don't catch the "Return, released"
	 * event that started this command. */
	fprintf(stderr, "Waiting for evdev data on stdin ...\n");

	//f = popen("arecord -D plughw:1,0 -f cd -r 44100 -c 1", "r");
	f = popen("parec --rate=44100 --format=s16le --channels=1 --latency=2", "r");
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
				snprintf(cmd, sizeof cmd, "sox -qq -r 44100 -e signed -b 16 -c 1 -t raw - %s", fname);
				printf("%02x %d: ", event.code, event.value);
				fflush(stdout);
				fout = popen(cmd, "w");
				triggered = 1;
				printf("%02x %d ", event.code, event.value);
				fflush(stdout);
			}
		}

		if (triggered)
		{
			size_t history_i = 0;
			printf(">");
			fflush(stdout);

			/* Write history. */
			for (history_i = 0; history_i < BUF_HIST; history_i++)
			{
				hist_at++;
				hist_at %= BUF_HIST;
				fwrite(&hist[hist_at], sizeof (int16_t), 1, fout);
			}

			/* Record additional samples. */
			fread(&post, BUF_POST, 1, f);
			fwrite(&post, BUF_POST, 1, fout);

			/* Close pipe and play back for examination. */
			fclose(fout);
			snprintf(cmd, sizeof(cmd), "paplay %s &", fname);
			system(cmd);
			printf(".\n");
			fout = NULL;
			triggered = 0;
			hist_at = 0;
		}

		if(FD_ISSET(fd_snd, &fds)) {
			/* Audio data ready to read but we haven't registered a key,
			 * so add those two bytes to our history. */
			read(fd_snd, &hist[hist_at], sizeof (int16_t));
			hist_at++;
			hist_at %= BUF_HIST;
		}

	}

	return 0;
}

