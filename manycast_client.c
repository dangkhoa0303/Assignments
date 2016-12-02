#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include "ntp_helper.h"
#include "reference.h"

int SERVER_PORT = 12345; // initialized as default

void setMessage(struct sntpMessageFormat *, struct timeval);

int main(int argc, char * argv[]) {
    int sockfd, numbytes = 0;
    struct sockaddr_in serv_addr;
    struct ip_mreq mreq;

    struct sntpMessageFormat recvMsg, sentMsg;
    struct timeval mTime;

    socklen_t addr_len = (socklen_t) sizeof(struct sockaddr);

    char *inputIP;

    /* must have at least 1 argument input */
    if (argc < 3) {
        perror("not enough arguments");
        exit(1);
    }
    /* first argument is the program name */

    /* second argument is input port number */
    int i = 0, NEW_PORT = 0;
    while (argv[1][i] != '\0') {
        NEW_PORT *= 10;
        NEW_PORT += argv[1][i] - '0';
        i++;
    }
    // set LISTEN_PORT
    SERVER_PORT = NEW_PORT;
    printf("SERVER PORT: %d\n", SERVER_PORT);

    /* third argument is the input ip address */
    inputIP = malloc(sizeof(argv[2]));
    strcpy(inputIP, argv[2]);
    printf("IP address: %s\n", inputIP);

    /* open socket */
    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket error");
        exit(1);
    }

    // zero struct
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = PF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    mreq.imr_multiaddr.s_addr = inet_addr(inputIP);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (inet_pton(PF_INET, inputIP, &serv_addr.sin_addr) < 0) {
	perror("pton error");
	exit(1);
    }

    /* Join the multicast group that receives datagrams */
    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("setsockopt error");
        exit(1);
    }

    /* set timeout for listening */
    struct timeval timeout;
    /* 
      After 5 seconds, if no message received, throw exception for timeout.
      When timeout, errno is set to EAGAIN or EWOULDBLOCK (according to Linux manpage)
    */
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("timeout error");
	exit(1);
    }

    /* initialize all fields to be 0 */
    initializeMessageFormat(&recvMsg);
    initializeMessageFormat(&sentMsg);

    gettimeofday(&mTime, NULL);
    setMessage(&sentMsg, mTime);
    convertByteOrder(&sentMsg);
        
    if ((numbytes = sendto(sockfd, &sentMsg, sizeof(sentMsg), 0,
         (struct sockaddr *) &serv_addr, addr_len)) < 0) {
        perror("sendto error");
        exit(1);
    }
    printf("Sent message to server\n");

        if ((numbytes = recvfrom(sockfd, (struct sntpMessageFormat *) &recvMsg,
                sizeof(struct sntpMessageFormat), 0, (struct sockaddr *) &serv_addr, &addr_len)) < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("Timeout: No message received\n");
                exit(1);
            } else {
                perror("recvfrom error");
                exit(1);
            }
        }
	gettimeofday(&mTime, NULL);
        printf("Received message from server\n");

        if (numbytes == sizeof(struct sntpMessageFormat)) {

            convertByteOrder(&recvMsg);
	        recvMsg.recvTimestamp = time2ntp(mTime);

            int mode  = recvMsg.li_vn_mode & 4; // server mode - 4
            int serverVersion = (recvMsg.li_vn_mode >> 3) & 0x07;

            if (mode != 4) {
                printf("Server mode is incorrect");
            } else if (serverVersion != 4) {
                printf("Server version is incorrect\n");
            } else if (recvMsg.stratum == 0) {
                printf("Kiss o' death message");
            } else {
                printf("Message received: \n");
                printMessageDetails(recvMsg, inputIP);
            }
        } else {
            printf("No message received\n");
        }
        
    close(sockfd);
    return 0;
}

void setMessage(struct sntpMessageFormat *msg, struct timeval mTime) {
    // LI
    msg->li_vn_mode = 0;

    // VN
    msg->li_vn_mode <<= 3; // shift left 3 bits
    // version number is 4
    msg->li_vn_mode |= 4;

    // MODE
    msg->li_vn_mode <<= 3;
    // I am a client
    msg->li_vn_mode |= 3; 

    msg->originateTimestamp = time2ntp(mTime);
    msg->transmitTimestamp = time2ntp(mTime);
}
