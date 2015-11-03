/*
 * This a library for a reliable UDP protocol.
 * The function call is same as the system defined
 * socket library. The prototype of the function is
 * same as system defined functions. The Errors set
 * by these functions are also same as the error set
 * by the library functions.
 *
 * All the data structures are maintained at the user
 * level apart from all the data structures used by
 * the system library of UDP protocol.
 *
 *
 * */

#ifndef RSOCKET_H_
#define RSOCKET_H_

#include<sys/types.h>

#define SOCK_BRP //TODO: allocate a number to the value

/*
 * r_socket is just like a socket() call which returns a
 * socket descriptor for a socket of basic reliable UDP
 * socket if the type is same as SOCK_BRP else it returns
 * what a simple socket call returns.
 *
 * */
int r_socket(int domain, int type, int protocol);

/*
 * r_sendto sends the data using the underlying UDP
 * sockets in a reliable manner. This functions guarantees
 * the packet delivery.
 *
 * */
ssize_t r_sendto(int sockfd, const void *buf, size_t len, int flags,
		const struct sockaddr *dest_addr, socklen_t addrlen);

/*
 * r_recvfrom receives data from a peer. Which sends packets
 *
 *
 * */
ssize_t r_recvfrom(int sockfd, void *buf, size_t len, int flags,
		struct sockaddr *src_addr, socklen_t *addrlen);

#endif
