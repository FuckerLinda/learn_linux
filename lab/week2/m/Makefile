main: main.o hello1.o hello2.o
		gcc -o main main.o hello1.o hello2.o
main.o: main.c hello1.h hello2.h
		gcc -c main.c
hello1.o: hello1.c hello1.h
		gcc -c hello1.c
hello2.o: hello2.c hello2.h
		gcc -c hello2.c
clean:
		rm -f main hello1.o hello2.o main.o
