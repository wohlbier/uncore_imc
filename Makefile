CC=icc
CFLAGS=-O0 -g -fasm-blocks -std=gnu99
EXE=uncore_imc
LDFLAGS=-static-intel
OBJ=uncore_imc.o counter.o


all : $(OBJ)
	$(CC) -o $(EXE) $(OBJ) $(LDFLAGS)

%.o : %.c
	$(CC) $(CFLAGS) -c $^

clean :
	-$(RM) *~ $(OBJ) $(EXE)
