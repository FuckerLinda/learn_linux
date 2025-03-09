LIB = libfunc.a
OBJ = func.o

all: $(LIB)

$(LIB): $(OBJ)
	ar rcs $@ $^

func.o: func.c
	gcc -c $< -o $@

clean:
	rm -f $(OBJ) $(LIB)
