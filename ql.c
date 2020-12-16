#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INBUF_SIZE 256
char inbuf[INBUF_SIZE];

char **symbols = NULL;
size_t nsymbols = 0;
size_t msymbols = 0;

int *sym_list = NULL;
size_t nsym_list = 0;
size_t msym_list = 0;

int push_symbol(char *symbol) {
	for (int i = 0; i < nsymbols; i++) {
		if (strcmp(symbol, symbols[i]) == 0) return i;
	}
	if (nsymbols == msymbols) {
		int new_count = msymbols ? msymbols * 2 : 8;
		if (!(symbols = realloc(symbols, new_count * sizeof(char*)))) {
			perror("realloc");
			exit(1);
		}
		msymbols = new_count;
	}
	if (!(symbols[nsymbols] = strdup(symbol))) {
		perror("strdup");
		exit(1);
	}
	return nsymbols++;
}

int push_sym_list(char * symbol) {
	int sym = push_symbol(symbol);
	if (nsym_list == msym_list) {
		int new_count = msym_list ? msym_list * 2 : 8;
		if (!(sym_list = realloc(sym_list, new_count * sizeof(int)))) {
			perror("realloc");
			exit(1);
		}
		msym_list = new_count;
	}
	sym_list[nsym_list++] = sym;
	return sym;
}

int process_chunk(int bytes_read, int last_chunk) {
	int i, j;
	for (i = 0, j = 0; i < bytes_read; i++) {
		char c = inbuf[i];
		if (isspace(c) || c == '(' || c == ')') {
			if (i != j) {
				inbuf[i] = 0;
				push_sym_list(inbuf+j);
			}
			if (c == '(') push_sym_list("(");
			if (c == ')') push_sym_list(")");
			j = i+1;
		}
	}
	if (last_chunk) {
		if (i != j) {
			inbuf[i] = 0;
			push_sym_list(inbuf+j);
		}
		i = 0;
	} else {
		for (i = 0; j + i < bytes_read; i++) {
			inbuf[i] = inbuf[j + i];
		}
	}
	return i;
}

void process_input() {
			size_t start, bytes_read;
 	for (
			start = 0, bytes_read = fread(inbuf, 1, INBUF_SIZE - 1, stdin);
			bytes_read > 0;
			bytes_read = fread(inbuf + start, 1, INBUF_SIZE - start - 1, stdin)
	) {
		if (ferror(stdin)) {
			perror("fread");
			exit(1);
		}
		start = process_chunk(start + bytes_read, 0);
		if (start == INBUF_SIZE - 1) {
			fprintf(stderr, "Error: token too long, maximum token length is %d\n", INBUF_SIZE - 2);
		}
	}
	if (ferror(stdin)) {
		perror("fread");
		exit(1);
	}
	process_chunk(start + bytes_read, 1);
}

int main() {
	process_input();
	for (int i = 0; i < nsym_list; i++) {
		printf("%s\n", symbols[sym_list[i]]);
	}
	return 0;
}
