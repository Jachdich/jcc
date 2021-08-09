SOURCES := $(shell find src -type f -name *.c)
HEADERS := $(shell find include -type f -name *.h)
OBJECTS := $(patsubst src/%,obj/%,$(SOURCES:.c=.o))

jcc: $(OBJECTS) lib/libjas.a
	gcc $(OBJECTS) -Llib -ljas -o $@

obj/%.o: src/%.c $(HEADERS) $(ASMHEADERS)
	@mkdir -p obj
	gcc -c -o $@ $< -Wall -g -Iinclude -Iassembler/include

lib/libjas.a:
	make -C assembler/ libjas.a
	cp assembler/libjas.a lib

debug: jcc
	gdb jcc

run: jcc
	./jcc test.jc

clean:
	rm obj/*.o
	rm jcc

install: jcc
	make -C assembler/
	cp assembler/jas .
	make -C linker/
	cp linker/jld .
	make -C vm/
	cp vm/jvm .
	mv jcc jas jld jvm /usr/local/bin

.PHONY: clean
.PHONY: install