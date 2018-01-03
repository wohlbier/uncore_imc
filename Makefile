EXE=uncore_imc
LDFLAGS=
OBJ=uncore_imc.o counter.o

CC=icc
CFLAGS=-O -g -fasm-blocks -std=gnu99

all : $(OBJ)
	$(CC) -o $(EXE) $(OBJ) $(LDFLAGS)

%.o : %.c
	$(CC) $(CFLAGS) -c $^

clean :
	-$(RM) *~ $(OBJ) $(EXE)
