CFLAGS = -Wall
TESTS := $(shell find tests -name *.in | sed 's/.in$$/.diff/')
OBJS = ql.o eval.o

ql: $(OBJS)
	cc -Wall $(OBJS) -o ql

clean:
	rm *.o ql tests/*.diff

tests/%.diff: ql tests/%.in tests/%.out
	@echo $@
	@./ql < tests/$*.in 2>&1 | diff tests/$*.out -

test:	$(TESTS)
