#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include <unistd.h>
#include"rsocket.h"
#include"dropMessage.h"

int r_socket(int domain, int type, int protocol) {
	//Native Implementation
	return socket(domain, type, protocol);
}

int r_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	//Native Implementation
	return bind(sockfd, addr, addrlen);
}

ssize_t r_sendto(int sockfd, const void *buf, size_t len, int flags,
		const struct sockaddr *dest_addr, socklen_t addrlen) {
	//Native Implementation
	return sendto(sockfd, buf, len, flags, dest_addr, addrlen);
}

ssize_t r_recvfrom(int sockfd, void *buf, size_t len, int flags,
	struct sockaddr *src_addr, socklen_t *addrlen) {
	//r_recvfrom can drop packets now
	/*start set temp variables for receiving packet*/
	char* curBuf = (char*)malloc(sizeof(char)*len);
	struct sockaddr curSrc_addr;
	memcpy(&curSrc_addr,src_addr,sizeof(struct sockaddr));
	/*end set temp variables for receiving packet*/

	/*Receive packet*/
	int retVal = recvfrom(sockfd, curBuf, len, flags, &curSrc_addr, addrlen);
	int prob = dropMessage(DROP_PROB);
	if(!prob){
		memcpy(buf,curBuf,retVal);
		memcpy(src_addr,&curSrc_addr,sizeof(struct sockaddr));
		free(curBuf);
		return retVal;
	}
	free(curBuf);
	return (retVal<0)?retVal:0;
}

int r_close(int fd) {
	//Native Implementation
	return close(fd);
}
