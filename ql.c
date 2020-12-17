#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INBUF_SIZE 256
char inbuf[INBUF_SIZE];

char *punctuation[] = {
	"(",
	")",
	"'"
};
char *builtins[] = {
	"quote",
	"hd",
	"tl",
	"cons",
	"cond",
	"atom",
	"eq",
	"lambda"
};
// order needs to match above
enum _builtins {
	LP,
	RP,
	Q,
	QUOTE,
	HD,
	TL,
	CONS,
	COND,
	ATOM,
	EQ,
	LAMBDA
};

typedef struct _pair {
	int hd;
	int tl;
} pair;

pair *pairs = NULL;
size_t npairs = 0;
size_t mpairs = 0;

char **symbols = NULL;
size_t nsymbols = 0;
size_t msymbols = 0;

int *sym_list = NULL;
size_t nsym_list = 0;
size_t msym_list = 0;

int cons(int hd, int tl) {
	if (npairs == mpairs) {
		int new_count = mpairs ? mpairs * 2 : 8;
		if (!(pairs = realloc(pairs, new_count * sizeof(pair)))) {
			perror("realloc");
			exit(1);
		}
		mpairs = new_count;
	}
	pairs[npairs].hd = hd;
	pairs[npairs].tl = tl;
	return -++npairs;
}

int hd(int p) {
	if (p < 0 && -1 - p < npairs) {
		return pairs[-1 - p].hd;
	}
	fprintf(stderr, "cannot take hd of non-cons\n");
	exit(1);
	return 0;
}

int tl(int p) {
	if (p < 0 && -1 - p < npairs) {
		return pairs[-1 - p].tl;
	}
	fprintf(stderr, "cannot take tl of non-cons\n");
	exit(1);
	return 0;
}

int quote(int e) {
	return cons(QUOTE, cons(e, 0));
}

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
		char *punc_str = NULL;
		for (int p = 0; p < sizeof(punctuation)/sizeof(char *); p++) {
			if (c == punctuation[p][0]) {
				punc_str = punctuation[p];
				break;
			}
		}
		if (isspace(c) || punc_str) {
			if (i != j) {
				inbuf[i] = 0;
				push_sym_list(inbuf+j);
			}
			if (punc_str) push_sym_list(punc_str);
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

char *to_s(int);

char *p_to_s(int e) {
	if (e == 0) {
		return strdup("");
	} else if (e > 0) {
		// " . " + to_s(e)
		char *es = to_s(e);
		int les = strlen(es);
		int len = les + 4;
		char *res = malloc(len);
		res[0] = ' ';
		res[1] = '.';
		res[2] = ' ';
		strcpy(res+3, es);
		free(es);
		return res;
	} else {
		// " " + to_s(hd(e)) + p_to_s(tl(e))
		char *hds = to_s(hd(e));
		char *tls = p_to_s(tl(e));
		int lhds = strlen(hds);
		int ltls = strlen(tls);
		int len = 2 + lhds + ltls;
		char *res = malloc(len);
		res[0] = ' ';
		strcpy(res+1, hds);
		strcpy(res+1+lhds, tls);
		free(hds);
		free(tls);
		return res;
	}
}

char *to_s(int e) {
	if (e == 0) {
		return strdup("()");
	} else if (e > 0) {
		return strdup(symbols[e]);
	} else {
		// "(" + to_s(hd(e)) + p_to_s(tl(e)) + ")"
		char *hds = to_s(hd(e));
		char *tls = p_to_s(tl(e));
		int lhds = strlen(hds);
		int ltls = strlen(tls);
		int len = 3 + lhds + ltls;
		char *res = malloc(len);
		res[0] = '(';
		strcpy(res+1, hds);
		strcpy(res+lhds+1, tls);
		res[len-2] = ')';
		res[len-1] = 0;
		free(hds);
		free(tls);
		return res;
	}
}

struct parse_t {
	int value;
	int next_token;
};

struct parse_t parse_exp(int next_token);

struct parse_t parse_lst(int next_token) {
	struct parse_t r = {0, next_token + 1};
	if (next_token >= nsym_list) {
		fprintf(stderr, "unmatched `(`\n");
		exit(1);
	}
	int t = sym_list[next_token];
	if (t == RP) {
		return r;
	}
	if (t == LP) {
		struct parse_t hd = parse_lst(next_token+1); 
		struct parse_t tl = parse_lst(hd.next_token);
		r.value = cons(hd.value, tl.value);
		r.next_token = tl.next_token;
		return r;
	}
	if (t == Q) {
		struct parse_t e = parse_exp(next_token+1); 
		struct parse_t tl = parse_lst(e.next_token);
		r.value = cons(quote(e.value), tl.value);
		r.next_token = tl.next_token;
		return r;
	}
	struct parse_t e = parse_lst(next_token + 1);
	r.value = cons(t, e.value);
	r.next_token = e.next_token;
	return r;
}

struct parse_t parse_exp(int next_token) {
	struct parse_t r = {0, next_token + 1};
	if (next_token >= nsym_list) {
		r.next_token = -1;
		return r;
	}
	int t = sym_list[next_token];
	if (t == LP) return parse_lst(next_token+1);
	if (t == RP) {
		fprintf(stderr, "unmatched `)`\n");
		exit(1);
	}
	if (t == Q) {
		struct parse_t e = parse_exp(next_token+1); 
		r.value = quote(e.value);
		r.next_token = e.next_token;
		return r;
	}
	r.value = t;
	return r;
}

void init_symbols() {
	for (int i = 0; i < sizeof(punctuation)/sizeof(char *); i++)
		push_symbol(punctuation[i]);
	for (int i = 0; i < sizeof(builtins)/sizeof(char *); i++)
		push_symbol(builtins[i]);
}

extern int eval(int e);

int main() {
	init_symbols();

	process_input();

	for (struct parse_t p = parse_exp(0); p.next_token != -1; p = parse_exp(p.next_token)) {
		int val = eval(p.value);
		printf("%s\n", to_s(val));
	}

	return 0;
}
