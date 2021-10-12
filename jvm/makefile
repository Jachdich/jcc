SOURCES := $(shell find src -type f -name *.c)
HEADERS := $(shell find include -type f -name *.h)
OBJECTS := $(patsubst src/%,obj/%,$(SOURCES:.c=.o))
EXENAME := jvm

$(EXENAME): $(OBJECTS)
	gcc $(OBJECTS) -ljtools -o $@

obj/%.o: src/%.c $(HEADERS)
	@mkdir -p obj
	gcc -c -o $@ $< -Wall -g -Iinclude

debug: $(EXENAME)
	gdb $(EXENAME)

run: $(EXENAME)
	./jvm
	
clean:
	rm -rf obj/*.o
	rm -rf $(EXENAME)

install: $(EXENAME)
	cp $(EXENAME) /usr/local/bin

.PHONY: clean
