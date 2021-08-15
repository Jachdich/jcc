SOURCES := $(shell find src -type f -name *.c)
HEADERS := $(shell find include -type f -name *.h)
OBJECTS := $(patsubst src/%,obj/%,$(SOURCES:.c=.o))
EXENAME := jld

$(EXENAME): $(OBJECTS)
	gcc $(OBJECTS) -ljobj -o $@

libjld.a: $(OBJECTS)
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
	rm obj/*.o
	rm $(EXENAME)

libinclude: $(HEADERS)
	mkdir -p libinclude
	rm -rf libinclude/*
	cp include/linker.h libinclude/jld.h

install: $(EXENAME) libjld.a libinclude
	cp $(EXENAME) /usr/local/bin
	cp libjld.a /usr/local/lib
	rm -rf /usr/local/include/jld
	cp -r libinclude /usr/local/include/jld
	
.PHONY: clean
.PHONY: install
