SOURCES := $(shell find src -type f -name *.c)
HEADERS := $(shell find include -type f -name *.h)
OBJECTS := $(patsubst src/%,obj/%,$(SOURCES:.c=.o))

jcc: $(OBJECTS)
	gcc $(OBJECTS) -ljas -ljobj -o $@

obj/%.o: src/%.c $(HEADERS) $(ASMHEADERS)
	@mkdir -p obj
	gcc -c -o $@ $< -Wall -gdwarf-3 -Iinclude -I../assembler/include

debug: jcc
	gdb jcc

run: jcc
	./jcc test.jc

runprog: jcc test.jc
	./jcc -o test.asm test.jc
	jas test.asm -o test.o
	jld test.o io.o -o test
	jvm test

clean:
	rm obj/*.o
	rm jcc

install: jcc
	cp jcc /usr/local/bin

.PHONY: clean
.PHONY: install
