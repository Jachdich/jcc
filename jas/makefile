SOURCES := $(shell find src -type f -name *.c)
HEADERS := $(shell find include -type f -name *.h)
OBJECTS := $(patsubst src/%,obj/%,$(SOURCES:.c=.o))
EXENAME := jas

$(EXENAME): $(OBJECTS)
	gcc $(OBJECTS) -ljtools -o $@

libjas.a: $(OBJECTS)
	ar -rc $@ $(filter-out obj/args.o obj/main.o,$(OBJECTS))
	ranlib $@

obj/%.o: src/%.c $(HEADERS)
	@mkdir -p obj
	gcc -c -o $@ $< -Wall -g -Iinclude

debug: $(EXENAME)
	gdb $(EXENAME)


run: $(EXENAME)
	./$(EXENAME)
	
clean:
	rm -rf obj/*.o
	rm -rf $(EXENAME)

libinclude: $(HEADERS)
	mkdir -p libinclude
	rm -rf libinclude/*
	cp include/assembler.h libinclude/jas.h

install: $(EXENAME) libjas.a libinclude
	cp $(EXENAME) /usr/local/bin
	cp libjas.a /usr/local/lib
	rm -rf /usr/local/include/jas
	cp -r libinclude /usr/local/include/jas
	
.PHONY: clean
.PHONY: install
