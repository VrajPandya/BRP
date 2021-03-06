/*
Licensed under creative commons 4.0.
License: Please see the "CREATIVE COMMONS LICENSE" file.
*/
/*
 * The user1 keeps sending a string of data to the user2
 * using BRP sockets.
 * */

#include"rsocket.h"
#include"user.h"
#include<stdio.h>
#include<sys/socket.h>
#include<string.h>
#include<netinet/in.h>
#include<stdlib.h>

static int user1portNum = USER1PORT;
static int user2portNum = USER2PORT;

int main(int argc, const char* argv[]) {

	struct sockaddr_in user1Addr, user2Addr;
	int sock, i, slen = sizeof(user2Addr);
	char* server = "127.0.0.1"; /* change this to use a different server */
	int bufSize = BRP_MTU;
	char* buf;
	int retVal;

	/*
	 *  get the optional arguments according to the number of arguments
	 * */
	if(argc >= 2){
		server = argv[1];
	}

	/* create a socket */

	if ((sock = r_socket(AF_INET, SOCK_BRP, 0)) == -1)
		printf("socket created\n");

	/* to bind the socket to all local addresses and described protocol */
	memset((char *) &user1Addr, 0, sizeof(user1Addr));
	user1Addr.sin_family = AF_INET;
	user1Addr.sin_addr.s_addr = htonl(INADDR_ANY);
	user1Addr.sin_port = user1portNum;

	/* bind the socket with given port and address */
	if (r_bind(sock, (struct sockaddr *) &user1Addr, sizeof(user1Addr)) < 0) {
		perror("bind failed");
		return 0;
	}

	/* now define user2Addr, the address to whom we want to send messages */
	/* For convenience, the host address is expressed as a numeric IP address */
	/* that we will convert to a binary format via inet_aton */
	memset((char *) &user2Addr, 0, sizeof(user2Addr));
	user2Addr.sin_family = AF_INET;
	user2Addr.sin_port = htons(user2portNum);
	if (inet_aton(server, &user2Addr.sin_addr) == 0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}

	/* set the buf*/
	buf = (char*) malloc(bufSize);
	memset(buf, 0, bufSize);

	/* go into infinite loop to and get the bytes to send */
	while (1) {
		printf("enter the message you wish to send:\n");
		fgets(buf, bufSize - 1, stdin);
		/*fragment each byte in the message and send it using BRP socket*/
		for (i = 0; i < strlen(buf); i++) {
			if (r_sendto(sock, &buf[i], 1, 0, (struct sockaddr *) &user2Addr,
					slen) == -1)
				perror("sendto");
		}
		memset(buf, 0, bufSize);
	}
	r_close(sock);
	return 0;
}
