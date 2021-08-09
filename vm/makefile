SOURCES := $(shell find src -type f -name *.c)
HEADERS := $(shell find include -type f -name *.h)
OBJECTS := $(patsubst src/%,obj/%,$(SOURCES:.c=.o))
EXENAME := jvm

$(EXENAME): $(OBJECTS)
	gcc $(OBJECTS) -o $@

obj/%.o: src/%.c $(HEADERS)
	@mkdir -p obj
	gcc -c -o $@ $< -Wall -O3 -Iinclude

debug: $(EXENAME)
	gdb $(EXENAME)


run: $(EXENAME)
	./jvm
	
clean:
	rm obj/*.o
	rm $(EXENAME)

.PHONY: clean
