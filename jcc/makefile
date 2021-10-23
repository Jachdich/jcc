SOURCES := $(shell find src -type f -name "*.c")
HEADERS := $(shell find include -type f -name "*.h")
OBJECTS := $(patsubst src/%,obj/%,$(SOURCES:.c=.o))

jcc: $(OBJECTS)
	@echo $(SOURCES)
	gcc $(OBJECTS) -ljas -ljtools -o $@

obj/%.o: src/%.c $(HEADERS) $(ASMHEADERS)
	@mkdir -p obj
	gcc -c -o $@ $< -Wall -gdwarf-3 -Iinclude

debug: jcc
	gdb jcc

run: jcc
	./jcc test.c

runprog: jcc test.jc
	./jcc -o test.asm test.c
	jas test.asm -o test.o
	jld test.o io.o -o test
	jvm test

clean:
	rm -rf obj/*.o
	rm -rf jcc

install: jcc
	cp jcc /usr/local/bin

.PHONY: clean
.PHONY: install
