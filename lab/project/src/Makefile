all: ../build/main

../build/main: main.o
	gcc -o $@ $< -L../lib -lfunc -lm

main.o: main.c
	gcc -I../lib -c $< -o $@

clean:
	rm -f main.o ../build/main
