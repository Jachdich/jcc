SOURCES := $(shell find src -type f -name *.c)
HEADERS := $(shell find include -type f -name *.h)
OBJECTS := $(patsubst src/%,obj/%,$(SOURCES:.c=.o))

libjobj.a: $(OBJECTS)
	ar -rc $@ $(OBJECTS)
	ranlib $@

obj/%.o: src/%.c $(HEADERS)
	@mkdir -p obj
	gcc -c -o $@ $< -Wall -g -Iinclude

clean:
	rm obj/*.o
	rm libjobj.a

libinclude: $(HEADERS)
	mkdir -p libinclude
	rm -rf libinclude/*
	cp include/jobj.h libinclude/

install: libjobj.a libinclude
	cp libjobj.a /usr/local/lib
	rm -rf /usr/local/include/jobj
	cp -r libinclude /usr/local/include/jobj

.PHONY: clean
.PHONY: install
