#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include "ntp_helper.h"
#include "reference.h"

void setMessage(struct sntpMessageFormat*, struct timeval);

int main(int argc, char * argv[]) {
    int sockfd, numbytes = 0;
    struct sockaddr_in multi_addr;
    // address of IPv4 multicast group
    struct ip_mreq mreq;
    struct sntpMessageFormat recvMsg, sentMsg;
    struct timeval mTime;

    int addr_len = sizeof(struct sockaddr);

    int LISTEN_PORT = 12345; // initialized as default

    char *inputIP;

    /* must have at least 1 argument input */
    if (argc < 3) {
        perror("not enough arguments");
        exit(1);
    }
    
    /* first argument is input port number */
    int i = 0, NEW_PORT = 0;
    while (argv[1][i] != '\0') {
        NEW_PORT *= 10;
        NEW_PORT += argv[1][i] - '0'; // convert each character into integer
        i++;
    }
    // set LISTEN_PORT
    LISTEN_PORT = NEW_PORT;
    printf("PORT NUMBER: %d\n", LISTEN_PORT);	

    /* second argument is the input ip address */
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
    memset(&multi_addr, 0, sizeof(multi_addr));

    multi_addr.sin_family = PF_INET;
    multi_addr.sin_port = htons(LISTEN_PORT);

    if (inet_pton(PF_INET, inputIP, &multi_addr.sin_addr) <= 0) {
        perror("pton error");
        exit(1);
    }

    /* bind */
    if (bind(sockfd, (struct sockaddr *) &multi_addr, sizeof(struct sockaddr)) < 0) {
        perror("bind error");
        exit(1);
    }

    mreq.imr_multiaddr.s_addr = inet_addr(inputIP);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    /* Join the multicast group with input IP address */
    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("setsockopt error");
        exit(1);
    }

    while (1) {
        printf("Listening...\n");
        initializeMessageFormat(&recvMsg);
        initializeMessageFormat(&sentMsg);

	/* Receive message from client */
        if ((numbytes = recvfrom(sockfd, (struct sntpMessageFormat *) &recvMsg,
                sizeof(struct sntpMessageFormat), 0, (struct sockaddr *) &multi_addr, &addr_len)) < 0) {
            perror("recvfrom error");
            exit(1);
        }
        convertByteOrder(&recvMsg);

        printf("Received from client\n");

        if (numbytes == sizeof(struct sntpMessageFormat)) {
            int mode = recvMsg.li_vn_mode & 3; // 3 is MODE set by client
            int version = (recvMsg.li_vn_mode >> 3) & 0x07;

	/* Check for mode, version number of the received message */
            if (mode != 3) {
                printf("%d is incorrect client mode", mode);
            } else if (version != 4) {
                printf("%d is incorrect version number", version);
            } else {
                sentMsg = recvMsg;
                gettimeofday(&mTime, NULL);
                setMessage(&sentMsg, mTime);
                
                // set reference timestamp
                gettimeofday(&mTime, NULL);
                sentMsg.refTimestamp = time2ntp(mTime);
                printMessageDetails(sentMsg, inputIP);

                convertByteOrder(&sentMsg);

                /* Reply to client */
                if ((numbytes = sendto(sockfd, (struct sntpMessageFormat *) &sentMsg,
                    sizeof(struct sntpMessageFormat), 0, (struct sockaddr *) &multi_addr, addr_len)) < 0) {
                    perror("sendto error");
                    exit(1);
                }
                printf("Replied to client\n\n");
            }
        } else {
            printf("Ignore this package due to incorrect size");
        }
    }
    close(sockfd);
    return 0;
}

void setMessage(struct sntpMessageFormat* msg, struct timeval mTime) {

    // LI - 2 bits
    msg->li_vn_mode = 0;

    // VN - 3 bits
    msg->li_vn_mode <<= 3;
    // version number is 4
    msg->li_vn_mode |= 4;

    // MODE
    msg->li_vn_mode <<= 3;
    // I am the server - replying to client
    msg->li_vn_mode |= 4; 

    msg->stratum = 1;
    msg->poll = 4;
    msg->precision = 4;
    msg->rootDelay = 0;
    msg->rootDispersion = 0;

    int i = 0;
    char refCode[] = "LOCL"; // uncalibrated local clock

    /* reference identifier is a bitstring */
    for (i=0; i<4; i++) {
	char temp = refCode[i] & 0xFF;
	msg->refIdentifier |= temp;
	if (i==3)
	    break;
	msg->refIdentifier = msg->refIdentifier << 8;
    }

    // time when server received request from client
    msg->recvTimestamp = time2ntp(mTime);
    // set transmit time in ntp format
    msg->transmitTimestamp = time2ntp(mTime);
    msg->originateTimestamp = msg->transmitTimestamp;
}
