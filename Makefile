ql: ql.c
	cc -Wall ql.c -o ql

test:	ql tests/*
	@for n in tests/*.in; do echo $$n; ./ql < $$n | diff tests/`basename $$n .in`.out -; done
