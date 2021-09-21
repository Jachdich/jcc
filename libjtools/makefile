SOURCES := $(shell find src -type f -name "*.c")
HEADERS := $(shell find include -type f -name "*.h")
OBJECTS := $(patsubst src/%,obj/%,$(SOURCES:.c=.o))

libjtools.a: $(OBJECTS)
	ar -rc $@ $(OBJECTS)
	ranlib $@

obj/%.o: src/%.c $(HEADERS)
	@mkdir -p obj
	gcc -c -o $@ $< -Wall -g -Iinclude

clean:
	rm -rf obj/*.o
	rm -rf libjtools.a

libinclude: $(HEADERS)
	mkdir -p libinclude
	rm -rf libinclude/*
	cp include/jobj.h include/reader.h libinclude/

install: libjtools.a libinclude
	cp libjtools.a /usr/local/lib
	rm -rf /usr/local/include/jtools
	cp -r libinclude /usr/local/include/jtools

.PHONY: clean
.PHONY: install
