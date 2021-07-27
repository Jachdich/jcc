SOURCES := $(shell find src -type f -name *.c)
HEADERS := $(shell find include -type f -name *.h)
OBJECTS := $(patsubst src/%,obj/%,$(SOURCES:.c=.o))

jcc: $(OBJECTS)
	gcc $(OBJECTS) -o $@

obj/%.o: src/%.c $(HEADERS)
	@mkdir -p obj
	gcc -c -o $@ $< -Wall -g -Iinclude

debug: jcc
	gdb jcc

run: server
	./jcc test.jc

clean:
	rm obj/*.o
	rm jcc

.PHONY: clean