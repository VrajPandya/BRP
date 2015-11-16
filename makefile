all: librsocket.a user1 user2 

librsocket.a: rsocket.o
	ar rcs librscoket.a rsocket.o dropMessage.o

rsocket.o: rsocket.c dropMessage.o
	gcc -static -c rsocket.c -O3 

dropMessage.o:
	gcc -c dropMessage.c -O3
	
user1: user1.o
	gcc -o user1 user1.o rsocket.o dropMessage.o -O3
	
user2: user2.o
	gcc -o user2 user2.o rsocket.o dropMessage.o -O3 
	
user1.o: user1.c 
	gcc -c user1.c -o user1.o -O3 

user2.o: user2.c 
	gcc -c user2.c -o user2.o -O3 

clean:
	rm -f user1.o user2.o user1 user2 librsocket.a rsocket.o dropMessage.o
	