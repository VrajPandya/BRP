/*
 * The user2 keeps receiving a string of data from the user1
 * using BRP sockets.
 * */

#include"../rsocket.h"
#include"user.h"
#include<stdio.h>
#include<sys/socket.h>
#include<string.h>
#include<netinet/in.h>
#include<stdlib.h>

static int user2portNum = USER2PORT;

int main(int argc, char* argv[]) {

	struct sockaddr_in user2Addr; /* our address */
	struct sockaddr_in user1Addr; /* remote address */
	socklen_t addrlen = sizeof(user1Addr); /* length of addresses */
	int recvlen; /* # bytes received */
	int sock; /* our socket */
	int BufSize = 1024;
	unsigned char buf[BufSize]; /* receive buffer */

	/* create a UDP socket */

	if ((sock = r_socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket\n");
		return 0;
	}

	/* bind the socket to any valid IP address and a specific port */

	memset((char *) &user2Addr, 0, sizeof(user2Addr));
	user2Addr.sin_family = AF_INET;
	user2Addr.sin_addr.s_addr = htonl(INADDR_ANY);
	user2Addr.sin_port = htons(user2portNum);

	if (r_bind(sock, (struct sockaddr *) &user2Addr, sizeof(user2Addr)) < 0) {
		perror("bind failed");
		return 0;
	}

	/* now loop, receiving data and printing what we received */
	for (;;) {
		printf("waiting on port %d\n", user2portNum);
		recvlen = r_recvfrom(sock, buf, BufSize, 0, (struct sockaddr *) &user1Addr,
				&addrlen);
		printf("received %d bytes\n", recvlen);
		if (recvlen > 0) {
			buf[recvlen] = 0;
			printf("received message: \"%s\"\n", buf);
		}
	}
	/* never exits */}
