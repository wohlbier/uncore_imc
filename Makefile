EXE=uncore_imc
LDFLAGS=
OBJ=uncore_imc.o counter.o

CC=icc
CC := tau $(CC)

CFLAGS=-O -g -fasm-blocks -std=gnu99
CFLAGS += -D__TAU_MANUAL_INST__

all : $(OBJ)
	$(CC) -o $(EXE) $(OBJ) $(LDFLAGS)

%.o : %.c
	$(CC) $(CFLAGS) -c $^

clean :
	-$(RM) *~ $(OBJ) $(EXE)
