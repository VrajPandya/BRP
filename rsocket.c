#include<stdlib.h>
#include <unistd.h>
#include"rsocket.h"

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
	//Native Implementation
	return recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
}

int r_close(int fd) {
	//Native Implementation
	return close(fd);
}
