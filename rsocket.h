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
#include<sys/socket.h>
#include<stdint.h>
#include<pthread.h>

#define DEBUG 1

#define SOCK_BRP 11 //a number assigned by the library for BRP protocol sockets

#define DROP_PKT 1 // todo if needed use preprocessor to remove the dropPacket fucntion

#define DROP_PROB 0.5
#define MAX_SOCK_NUM 25
#define THREAD_WAIT_TIME 1
#define RETRANS_TIME 3

#define PKT_ACK 1

#define BRP_MTU 1400 //todo need to change so it matches the best estimeted MTU
				 //of all the networks for now we assume all the packets
				 //are of less then the MTU

/*
 * BRP packet is a collection of the header and the user data which needs
 * to be sent by the user.
 * The BRP packet header is an header which is added to the original message
 * to add the features like reliability
 *
 * flags: it is a collection of bit sized data which is used to determine
 * 		  which kind of packet this is.
 *		  The numbers of the bits are from least significant to most significant
 *		  bit			use
 *		  1 		 	Is ACK bit. set to indicate the packet is ACK packet
 *
 *
 * secquenceNumber: 32 bit number represents a number which is to be sent.
 *
 *
 * */
typedef struct brpPkt{
	/*star BRP header*/
	uint8_t flags;
	uint32_t secquenceNumber;
	/*end BRP header*/
	struct sockaddr* addr;
	char* data;
	size_t datalen;
	int userSendtoFlags;
}brpPkt_t;

/*
 * BRP packet Entry is the information of the packet
 * */
typedef struct brpPktEntry{
	brpPkt_t* pkt;
	time_t lastupdate;
	struct brpPktEntry* next;
} brpPktEntry_t;

/*
 * The Socket information structure is used to keep track of current state of
 * the socket and all the other related data structures.
 * It is a linked List which keeps track of all the important information
 * like current sequence number
 * UnACKed packet table
 * and the next pointer
 *
 *
 * */
typedef struct brpSockInfo{
	int sock_fd;
	uint32_t curSecquenceNumber;
	struct brpPktEntry* unAckedpktTable;
	struct brpPktEntry* recvedMsgTable;
	struct brpSockInfo* next;
	pthread_t* S;
	pthread_t* R;
	pthread_mutex_t* unAckedPktTableMutex;
	pthread_mutex_t* recvedMsgTableMutex;
} brpSockInfo_t;




/*
 * r_socket is just like a socket() call which returns a
 * socket descriptor for a socket of basic reliable UDP
 * socket if the type is same as SOCK_BRP else it returns
 * what a simple socket call returns.
 *
 * */
extern int r_socket(int domain, int type, int protocol);

/*
 * r_bind binds a socket to any given name.
 * It is just as same as the bind call.
 *
 * */
extern int r_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

/*
 * r_sendto sends the data using the underlying UDP
 * sockets in a reliable manner. This functions guarantees
 * the packet delivery.
 *
 * */
extern ssize_t r_sendto(int sockfd, const void *buf, size_t len, int flags,
		const struct sockaddr *dest_addr, socklen_t addrlen);

/*
 * r_recvfrom receives data from a peer. Which sends ACK
 * packets back to the sender.
 *
 *
 * */
extern ssize_t r_recvfrom(int sockfd, void *buf, size_t len, int flags,
		struct sockaddr *src_addr, socklen_t *addrlen);

/*
 * r_close closes the socket
 * kills all threads and frees all memory associated
 * with the socket. If any data is there in the
 * received-message table, it is discarded.
 *
 **/
extern int r_close(int fd);

#endif
