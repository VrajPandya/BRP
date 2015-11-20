all: librsocket.a user1 user2 

librsocket.a: rsocket.o
	ar rcs librscoket.a rsocket.o dropMessage.o

rsocket.o: rsocket.c dropMessage.o
	gcc -g -static -c rsocket.c -O0 -lpthread

dropMessage.o:
	gcc -g -c dropMessage.c -O0
	
user1: user1.o
	gcc -g -o user1 user1.o rsocket.o dropMessage.o -O0 -lpthread
	
user2: user2.o
	gcc -g -o user2 user2.o rsocket.o dropMessage.o -O0 -lpthread
	
user1.o: user1.c 
	gcc -g -c user1.c -o user1.o -O0 

user2.o: user2.c 
	gcc -g -c user2.c -o user2.o -O0

clean:
	rm -f user1.o user2.o user1 user2 librsocket.a rsocket.o dropMessage.o
	