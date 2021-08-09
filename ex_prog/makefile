SOURCES := $(shell find src -type f -name *.jc)
HEADERS := $(shell find include -type f -name *.h)
OBJECTS := $(patsubst src/%,obj/%,$(SOURCES:.jc=.o))

main: $(OBJECTS)
	jld $(OBJECTS) -o $@

asm/%.jas: src/%.jc $(HEADERS)
	@mkdir -p asm
	jcc -o $@ $<

obj/%.o: asm/%.jas
	@mkdir -p obj
	jas -o $@ $<

run: main
	jvm main

clean:
	rm -rf obj/*.o
	rm -rf asm/*.jas
	rm -rf main

.PHONY: clean
