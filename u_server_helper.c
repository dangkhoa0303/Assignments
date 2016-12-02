#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include "ntp_helper.h"
#include "reference.h"
#include "u_server_helper.h"

void handle(int sockfd, struct sockaddr_in client_addr) {

    int numbytes = 0;
    int addr_len = sizeof(client_addr);
    int msgFormatSize = sizeof(struct sntpMessageFormat);

    struct timeval mTime;

    struct sntpMessageFormat sentMsg;
    struct sntpMessageFormat recvMsg;

    // set all fields to be 0
    initializeMessageFormat(&sentMsg);
    initializeMessageFormat(&recvMsg);

    printf("Waiting for request from client...\n");

    /* .......... Receive message from client .......... */
    if ((numbytes = recvfrom(sockfd, (struct sntpMessageFormat *) &recvMsg, 
            msgFormatSize, 0, (struct sockaddr *) &client_addr, &addr_len)) == -1) {
        perror("error recvfrom");
        exit(1);
    }

    // get current time
    gettimeofday(&mTime, NULL);
    // convert received message from Network Byte Order into Host Byte Order 
    convertByteOrder(&recvMsg);
    // set received time for received message (in Network Byte Order). So, use time2ntp
    recvMsg.recvTimestamp = time2ntp(mTime);
    
    printf("Received message from client\n");

    /* .......... Send message back to client .......... */
    sentMsg = recvMsg; // in Host Byte Order

    gettimeofday(&mTime, NULL);
    
    // set sntp format for replied message
    setMessage(&sentMsg, mTime);
    // convert message sent from Host Byte Order into Network Byte Order

    sentMsg.refTimestamp = time2ntp(mTime);

    convertByteOrder(&sentMsg);   

    if ((numbytes = sendto(sockfd, (struct sntpMessageFormat*) &sentMsg, 
            msgFormatSize, 0, (struct sockaddr *) &client_addr, addr_len)) == -1) {
        perror("error sendto");
        exit(1);
    }
    printf("Replied to client\n");
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
