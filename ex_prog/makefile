main: main.o
	gcc -no-pie main.o -o main

clean:
	rm -f main.s main main.o

run: main
	./main
	
.PHONY: run
.PHONY: clean

main.s: main.jc
	../jcc main.jc -o main.s

main.o: main.s
	nasm -felf64 main.s
