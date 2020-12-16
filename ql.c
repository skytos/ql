#include <stdio.h>
#include <stdlib.h>

#define INBUF_SIZE 8
char inbuf[INBUF_SIZE];

int main() {
	size_t bytes_written;
 	for (
			size_t bytes_read = fread(inbuf, 1, INBUF_SIZE-1, stdin);
			bytes_read > 0;
			bytes_read = fread(inbuf, 1, INBUF_SIZE-1, stdin)
	) {
		if (ferror(stdin)) {
			perror("fread");
			exit(1);
		}
		bytes_written = fwrite(inbuf, 1, bytes_read, stdout);
		if (ferror(stdout)) {
			perror("fwrite");
			exit(1);
		}
	}
	if (ferror(stdin)) {
		perror("fread");
		exit(1);
	}
	return 0;
}
