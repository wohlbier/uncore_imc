all :
	icc -O -g -fasm-blocks -std=c99 -c app.c
	icc -O -g -std=gnu99 counter.c -c
	icc -o counter counter.o app.o -static-intel

clean :
	-$(RM) *~ *.o counter
