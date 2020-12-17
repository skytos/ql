ql: ql.o eval.o
	cc -Wall ql.o eval.o -o ql

ql.o: ql.c
	cc -Wall -c ql.c
	
eval.o: eval.c
	cc -Wall -c eval.c

test:	ql tests/*
	@for n in tests/*.in; do echo $$n; ./ql < $$n 2>&1 | diff tests/`basename $$n .in`.out -; done
