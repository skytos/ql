extern int hd(int);
extern int tl(int);
extern int cons(int, int);
extern int quote(int);

// order needs to match ql.c
enum _builtins {
	QUOTE = 3,
	HD,
	TL,
	CONS,
	COND,
	ATOM,
	EQ,
	LAMBDA
};

int eval(int x) {
	return x;
}
