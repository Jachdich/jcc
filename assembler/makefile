SOURCES := $(shell find src -type f -name *.c)
HEADERS := $(shell find include -type f -name *.h)
OBJECTS := $(patsubst src/%,obj/%,$(SOURCES:.c=.o))
EXENAME := jas

$(EXENAME): $(OBJECTS)
	gcc $(OBJECTS) -L../objtools -lobj -o $@

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
	rm obj/*.o
	rm $(EXENAME)

.PHONY: clean
