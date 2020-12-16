ql: ql.c
	cc -Wall ql.c -o ql

test:	ql ql.c
	sed "s/\([()']\)/ \1 /g" ql.c | tr ' \011' '\012\012' | grep -v '^$$' > 1
	./ql < ql.c > 2
	diff 1 2
	rm 1 2
