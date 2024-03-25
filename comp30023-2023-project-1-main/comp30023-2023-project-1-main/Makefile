CC = gcc
CFLAGS = -Wall -g
EXE = allocate
OBJ = list.o pq.o proc.o

$(EXE): main.c $(OBJ)
	$(CC) $(CFLAGS) -o $(EXE) $< $(OBJ) -lm

%.o: %.c %.h
	$(CC) -c -o $@ $< $(CFLAGS)
clean:
	rm -f *.o $(EXE)

format:
	clang-format -style=file -i *.c


