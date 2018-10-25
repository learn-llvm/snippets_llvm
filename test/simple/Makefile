SRC = $(wildcard *.c)
OBJ = $(patsubst %.c, %.o, $(SRC))
BIN = a.out

all: $(OBJ)
	$(CC) $(OBJ) -o $(BIN)

%.bc : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.ll *.bc *.out *.o
