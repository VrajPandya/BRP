all: user1.o user2.o
	gcc -static -o user1 user1.o -O3 -L.. -lrsocket
	gcc -static -o user2 user2.o -O3 -L.. -lrsocket
	
user1.o: user2.c
	gcc -c user1.c -o user1.o -O3

user2.o: user1.c
	gcc -c user2.c -o user2.o -O3

clean:
	rm -f user1.o user2.o user1 user2
