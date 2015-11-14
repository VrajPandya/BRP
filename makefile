librsocket.a: rsocket.o
	ar rcs librscoket.a rsocket.o

rsocket.o:
	gcc -c rsocket.c -O3
	