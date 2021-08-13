SOURCES := $(shell find src -type f -name *.c)
HEADERS := $(shell find include -type f -name *.h)
OBJECTS := $(patsubst src/%,obj/%,$(SOURCES:.c=.o))

libobj.a: $(OBJECTS)
	ar -rc $@ $(OBJECTS)
	ranlib $@

obj/%.o: src/%.c $(HEADERS)
	@mkdir -p obj
	gcc -c -o $@ $< -Wall -g -Iinclude

clean:
	rm obj/*.o
	rm libobj.a

.PHONY: clean
