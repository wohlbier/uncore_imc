CC=icc
CFLAGS=-O -g -fasm-blocks -std=gnu99
EXE=counter
LDFLAGS=-static-intel
OBJ=app.o counter.o


all : $(OBJ)
	$(CC) -o $(EXE) $(OBJ) $(LDFLAGS)

%.o : %.c
	$(CC) $(CFLAGS) -c $^

clean :
	-$(RM) *~ $(OBJ) $(EXE)
