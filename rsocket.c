#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<time.h>
#include<pthread.h>
#include"rsocket.h"
#include"dropMessage.h"

/*
 * add the extra size of the packet headers
 *
 * 1 for 8 bit flags
 * 4 for 32 bit secquence numbers
 *
 * */
#define BRP_PKT_SIZE(L) L + 1 + 4

typedef struct threadArg {
	int sockFd;
} threadArg_t;

/*
 * The master record of all the sockets
 *
 *
 *
 * */
static brpSockInfo_t* masterRecord;

static brpPkt_t* createBrpPkt(u_int8_t flags, const char* data,
		const struct sockaddr* addr, size_t len, int userSendtoFlags) {
	struct sockaddr* tempAddr = 0;
	char* curdata;
	brpPkt_t* res = (brpPkt_t*) malloc(sizeof(brpPkt_t));
	res->datalen = len;
	res->flags = flags;
	res->userSendtoFlags = userSendtoFlags;
	curdata = (char*) malloc(len);
	if (addr != NULL) {
		tempAddr = (struct sockaddr*) malloc(sizeof(struct sockaddr));
		memcpy(tempAddr, addr, sizeof(struct sockaddr));
	}
	res->addr = tempAddr;
	memcpy(curdata, data, len);
	res->addr = tempAddr;
	res->data = curdata;
	return res;
}

static void freeBrpPkt(brpPkt_t* pkt) {
	free(pkt->addr);
	free(pkt->data);
	free(pkt);
}

static void freeBrpPktEntry(brpPktEntry_t* Entry) {
	freeBrpPkt(Entry->pkt);
	free(Entry);
}

static brpSockInfo_t* addBrpSockInfo(int sockfd) {
	brpSockInfo_t* temp = masterRecord;
	if (masterRecord == NULL) {
		masterRecord = (brpSockInfo_t*) malloc(sizeof(brpSockInfo_t));
		temp = masterRecord;
	} else {
		while (temp->next != NULL)
			temp = temp->next;

		temp->next = (brpSockInfo_t*) malloc(sizeof(brpSockInfo_t));
		temp = temp->next;
	}
	temp->sock_fd = sockfd;
	temp->next = NULL;
	temp->unAckedpktTable = NULL;
	temp->recvedMsgTable = NULL;
	temp->curSecquenceNumber = 0;
	return temp;
}

static void freeBrpSockInfo(brpSockInfo_t* Info) {
	brpPktEntry_t* temp;
	brpPktEntry_t* next;
	pthread_cancel(*Info->R);
	pthread_cancel(*Info->S);
	pthread_mutex_destroy(Info->recvedMsgTableMutex);
	pthread_mutex_destroy(Info->unAckedPktTableMutex);
	temp = Info->recvedMsgTable;
	while (temp != NULL) {
		next = temp->next;
		freeBrpPktEntry(temp);
		temp = next;
	}
	temp = Info->unAckedpktTable;
	while (temp != NULL) {
		next = temp->next;
		freeBrpPktEntry(temp);
		temp = next;
	}
	free(Info);
}

static brpSockInfo_t* getBrpSockInfo(int sockfd) {
	brpSockInfo_t* temp = masterRecord;
	if (temp == NULL) {
		return NULL;
	}
	while (temp != NULL) {
		if (temp->sock_fd == sockfd) {
			return temp;
		}
		temp = temp->next;
	}
	return NULL;
}

static int removeBrpSockInfo(int sockfd) {
	brpSockInfo_t* back_ptr = masterRecord;
	brpSockInfo_t* fwd_ptr;
	if (back_ptr == NULL) {
		return -1;
	}
	if (back_ptr->sock_fd == sockfd) {
		masterRecord = back_ptr->next;
		freeBrpSockInfo(back_ptr);
		return sockfd;
	} else {
		fwd_ptr = back_ptr->next;
		while (fwd_ptr->sock_fd != sockfd && fwd_ptr->next != NULL) {
			back_ptr = fwd_ptr;
			fwd_ptr = fwd_ptr->next;
		}
		if (fwd_ptr->sock_fd == sockfd) {
			back_ptr->next = fwd_ptr->next;
			freeBrpSockInfo(fwd_ptr);
			return sockfd;
		}
	}
	return -1;
}

static brpPktEntry_t* addBrpPktEntry(brpPkt_t* pkt, int sock) {
	brpSockInfo_t* sockInfo = getBrpSockInfo(sock);
	int seqNo = sockInfo->curSecquenceNumber;
	sockInfo->curSecquenceNumber = sockInfo->curSecquenceNumber + 1;
	brpPktEntry_t* temp = sockInfo->unAckedpktTable;
	if (temp == NULL) {
		sockInfo->unAckedpktTable = (brpPktEntry_t*) malloc(
				sizeof(brpPktEntry_t));
		temp = sockInfo->unAckedpktTable;
	} else {
		while (temp->next != NULL) {
			temp = temp->next;
		}
		temp->next = (brpPktEntry_t*) malloc(sizeof(brpPktEntry_t));
		temp = temp->next;
	}
	temp->lastupdate = time(NULL);
	temp->pkt = pkt;
	temp->pkt->secquenceNumber = seqNo;
	temp->next = NULL;
	return temp;
}

static brpPktEntry_t* getBrpPktEntry(int sock, u_int32_t secquenceNumber) {
	brpSockInfo_t* sockInfo = getBrpSockInfo(sock);

	brpPktEntry_t* temp = sockInfo->unAckedpktTable;
	if (temp == NULL) {
		return NULL;
	} else {
		while (temp != NULL) {
			if (temp->pkt->secquenceNumber == secquenceNumber) {
				return temp;
			}
			temp = temp->next;
		}
	}
	return NULL;
}

static u_int32_t removeBrpPktEntry(int sock, u_int32_t secquenceNumber) {
	brpSockInfo_t* sockInfo = getBrpSockInfo(sock);
	brpPktEntry_t* back_ptr = sockInfo->unAckedpktTable;
	brpPktEntry_t* fwd_ptr;
	if (back_ptr == NULL) {
		return 0;
	}
	if (back_ptr->pkt->secquenceNumber == secquenceNumber) {
		sockInfo->unAckedpktTable = back_ptr->next;
		freeBrpPktEntry(back_ptr);
		return secquenceNumber;
	} else {
		fwd_ptr = back_ptr->next;
		while (fwd_ptr->pkt->secquenceNumber != secquenceNumber
				&& fwd_ptr != NULL) {
			back_ptr = fwd_ptr;
			fwd_ptr = fwd_ptr->next;
		}
		if (fwd_ptr->pkt->secquenceNumber == secquenceNumber) {
			back_ptr->next = fwd_ptr->next;
			freeBrpPktEntry(fwd_ptr);
			return secquenceNumber;
		}
	}
	return 0;
}

static brpPktEntry_t* storeRecvedMsg(brpPkt_t* pkt, int sock) {
	brpSockInfo_t* sockInfo = getBrpSockInfo(sock);
	brpPktEntry_t* temp = sockInfo->recvedMsgTable;
	if (temp == NULL) {
		sockInfo->recvedMsgTable = (brpPktEntry_t*) malloc(
				sizeof(brpPktEntry_t));
		temp = sockInfo->recvedMsgTable;
	} else {
		while (temp->next != NULL) {
			temp = temp->next;
		}
		temp->next = (brpPktEntry_t*) malloc(sizeof(brpPktEntry_t));
		temp = temp->next;
	}
	temp->pkt = pkt;
	temp->next = NULL;
	return temp;
}

static brpPktEntry_t* getRecvedMsg(int sock) {
	brpSockInfo_t* sockInfo = getBrpSockInfo(sock);
	brpPktEntry_t* temp = sockInfo->recvedMsgTable;
	if (temp == NULL) {
		return NULL;
	} else {
		sockInfo->recvedMsgTable = temp->next;
		return temp;
	}
}

static char* createBrpPktData(brpPkt_t* pkt) {
	int curOffset = 0;
	char* res = (char*) malloc(BRP_PKT_SIZE(pkt->datalen));
	char* offsetPtr = res;
	memcpy(offsetPtr, &pkt->flags, sizeof(u_int8_t));
	offsetPtr += sizeof(u_int8_t);
	memcpy(offsetPtr, &pkt->secquenceNumber, sizeof(u_int32_t));
	offsetPtr += sizeof(u_int32_t);
	memcpy(offsetPtr, pkt->data, pkt->datalen);
	return res;
}

static void* S(void* arg) {
	threadArg_t* args = (threadArg_t*) arg;
	int sock = args->sockFd;
	brpPktEntry_t* pktEntry;
	brpPkt_t* pkt;
	char* pktData;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	free(arg);
	brpSockInfo_t* sockInfo = getBrpSockInfo(sock);
	pthread_mutex_t* unAckedTableMutex = sockInfo->unAckedPktTableMutex;
	while (1) {
		sleep(THREAD_WAIT_TIME);
		pthread_mutex_lock(unAckedTableMutex);
		pktEntry = sockInfo->unAckedpktTable;
		while (pktEntry != NULL) {
			if (time(NULL) - pktEntry->lastupdate > RETRANS_TIME) {
				pkt = pktEntry->pkt;
				/*create packet data*/
				pktData = (char*) createBrpPktData(pkt);

				/*resend the packet which needs to be resent*/
				sendto(sock, pktData, BRP_PKT_SIZE(pkt->datalen), pkt->flags,
						pkt->addr, sizeof(struct sockaddr)); // todo update the sizeof operator and insert something more "elegant"

				/*free the buffer used to hold the BRP packet data*/
				free(pktData);

				/*update retransmit*/
				pktEntry->lastupdate = time(NULL);
			}
			pktEntry = pktEntry->next;
		}
		pthread_mutex_unlock(unAckedTableMutex);
	}
	return NULL;
}

static int drop_recvfrom(int sockfd, void *buf, size_t len, int flags,
		struct sockaddr *src_addr, socklen_t *addrlen) {
	//r_recvfrom can drop packets now

	/*start set temp variables for receiving packet*/
	char* curBuf = (char*) malloc(sizeof(char) * len);
	struct sockaddr curSrc_addr;
	/*end set temp variables for receiving packet*/

	/*Receive packet*/
	int retVal = recvfrom(sockfd, curBuf, len, flags, &curSrc_addr, addrlen);
	int prob = dropMessage(DROP_PROB);
	if (!prob) {
		memcpy(buf, curBuf, retVal);
		memcpy(src_addr, &curSrc_addr, sizeof(struct sockaddr));
		free(curBuf);
		return retVal;
	}
	free(curBuf);
	return (retVal < 0) ? retVal : 0;
}

static void* R(void* arg) {
	char buf[BRP_MTU];
	char replyBuf[BRP_PKT_SIZE(0)];
	struct sockaddr srcAddr;
	brpPkt_t* pkt;
	threadArg_t* args = (threadArg_t*) arg;
	int sock = args->sockFd;
	socklen_t len = sizeof(struct sockaddr);
	uint8_t curPktflags;
	uint32_t curPktSecquenceNo;
	brpSockInfo_t* sockInfo = getBrpSockInfo(sock);
	pthread_mutex_t* unAckedPktTableMutex = sockInfo->unAckedPktTableMutex;
	pthread_mutex_t* recvedMsgTableMutex = sockInfo->recvedMsgTableMutex;
	brpPktEntry_t* unAckedPktTable = sockInfo->unAckedpktTable;
	brpPktEntry_t* recvedMsgTable = sockInfo->recvedMsgTable;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	free(arg);
	int brpFlag = PKT_ACK;
	memcpy(replyBuf, (int*) &brpFlag, sizeof(uint8_t));
	while (1) {

		recvfrom(sock, buf, BRP_MTU, 0, &srcAddr, &len);
		memcpy(&curPktflags, buf, sizeof(uint8_t));
		memcpy(&curPktSecquenceNo, buf + sizeof(uint8_t), sizeof(uint32_t));

		if (curPktflags & PKT_ACK) {
			memcpy(replyBuf + sizeof(uint8_t), &curPktSecquenceNo,
					sizeof(uint32_t));
			sendto(sock, replyBuf, BRP_PKT_SIZE(0), 0, &srcAddr,
					sizeof(struct sockaddr));
			pthread_mutex_lock(unAckedPktTableMutex);
			removeBrpPktEntry(sock, curPktSecquenceNo);
			pthread_mutex_unlock(unAckedPktTableMutex);
		} else {
			pkt = createBrpPkt(0, buf, &srcAddr, BRP_MTU, 0);
			pthread_mutex_lock(recvedMsgTableMutex);
			storeRecvedMsg(pkt, sock);
			pthread_mutex_unlock(recvedMsgTableMutex);
		}
	}

	return NULL;
}

extern int r_socket(int domain, int type, int protocol) {
	int retVal;
	if (type == SOCK_BRP) {
		retVal = socket(domain, SOCK_DGRAM, protocol);
		if (retVal >= 0) {
			/*start set basic basic data structures for the BRP socket*/
			brpSockInfo_t* sockInfo = addBrpSockInfo(retVal);

			/*start set mutex*/
			sockInfo->recvedMsgTableMutex = (pthread_mutex_t*) malloc(
					sizeof(pthread_mutex_t));
			sockInfo->unAckedPktTableMutex = (pthread_mutex_t*) malloc(
					sizeof(pthread_mutex_t));
			pthread_mutex_init(sockInfo->unAckedPktTableMutex, NULL);
			pthread_mutex_init(sockInfo->recvedMsgTableMutex, NULL);
			/*end set mutex*/

			/*start set threadID*/
			sockInfo->R = (pthread_t*) malloc(sizeof(pthread_t));
			sockInfo->S = (pthread_t*) malloc(sizeof(pthread_t));
			/*end set threadID*/

			/*start set thread arguments*/
			threadArg_t* sArg = (threadArg_t*) malloc(sizeof(threadArg_t));
			threadArg_t* rArg = (threadArg_t*) malloc(sizeof(threadArg_t));
			sArg->sockFd = retVal;
			rArg->sockFd = retVal;
			/*end set basic data structures for BRP socket*/

			/*start run threads S and R*/
			pthread_create(sockInfo->R, NULL, R, (void*) rArg);
			pthread_create(sockInfo->S, NULL, S, (void*) sArg);
			/*end run thread S and R*/

		}
		return retVal;
	}
	return socket(domain, type, protocol);

}

extern int r_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
//Native Implementation
	return bind(sockfd, addr, addrlen);
}

extern ssize_t r_sendto(int sockfd, const void *buf, size_t len, int flags,
		const struct sockaddr *dest_addr, socklen_t addrlen) {
	brpPkt_t* pktInfo;
	brpSockInfo_t* sockInfo = getBrpSockInfo(sockfd);
	/*start set extra headers*/
	u_int8_t brpFlag = 0; //Do not set the ACK header so all 0
	/*create a BRP packet data structure*/
	pktInfo = createBrpPkt(brpFlag, buf, dest_addr, len, flags);

	/*ensure thread safety*/
	pthread_mutex_lock(sockInfo->unAckedPktTableMutex);
	addBrpPktEntry(pktInfo, sockfd); //add packet entry
	pthread_mutex_unlock(sockInfo->unAckedPktTableMutex);
	/*end set extra headers*/
	return sendto(sockfd, buf, len, flags, dest_addr, addrlen);
}

extern ssize_t r_recvfrom(int sockfd, void *buf, size_t len, int flags,
		struct sockaddr *src_addr, socklen_t *addrlen) {
//r_recvfrom can drop packets now

	/*start set temp variables for receiving packet*/
	brpSockInfo_t* sockInfo = getBrpSockInfo(sockfd);
	pthread_mutex_t* mutex = sockInfo->recvedMsgTableMutex;
	brpPktEntry_t* pktEntry = 0;
	brpPkt_t* curPkt = 0;
	while (!pktEntry) {
		pthread_mutex_lock(mutex);
		pktEntry = getRecvedMsg(sockfd);
		pthread_mutex_unlock(mutex);
	}
	curPkt = pktEntry->pkt;
	len = curPkt->datalen;
	memcpy(buf, curPkt->data, len);
	if (src_addr != NULL)
		memcpy(src_addr, curPkt->addr, *addrlen);
	return 0;
}

extern int r_close(int fd) {
	int retVal;
	removeBrpSockInfo(fd);
	retVal = close(fd);
	return retVal;
}
