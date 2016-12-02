#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include "ntp_helper.h"
#include "reference.h"
#include "u_client_helper.h"

void requestMessage(int sockfd, struct sockaddr_in server_addr, char *IP) {
    int numbytes;
    int addr_len = sizeof(server_addr);

    struct timeval mTime;

    struct sntpMessageFormat msg;
    struct sntpMessageFormat recvMsg;

    /* get current time */
    gettimeofday(&mTime, NULL);

    /* set all fields to be 0 */
    initializeMessageFormat(&msg);

    /* set time message departed for server */
    setMessage(&msg, mTime);
    /* msg is in ntp format */
    convertByteOrder(&msg);

    /* Send message to client */
    if ((numbytes = sendto(sockfd, &msg, sizeof(msg), 0, 
        (struct sockaddr *) &server_addr, addr_len)) == -1) {
        perror("sendto error");
        exit(1);
    }
    printf("Sent request\n");
    
    /* Receive message from client */
    if ((numbytes = recvfrom(sockfd, (struct sntpMessageFormat *) &recvMsg,
            sizeof(struct sntpMessageFormat), 0, (struct sockaddr *) &server_addr,
            &addr_len)) == -1) {
	if (errno == EAGAIN || errno == EWOULDBLOCK) {
	    printf("Timeout: No message received\nResending message...\n\n");
        } else {
            perror("recvfrom error");
            exit(1);
        }
    } else {
	printf("Received from server\n");

        gettimeofday(&mTime, NULL);
        if (numbytes > 0) {
            int mode  = recvMsg.li_vn_mode & 4; // server mode - 4
	    /* Kiss o' death check */
            if (recvMsg.stratum == 0) {
                printf("Kiss o' death message");
	    /* Server mode check */
            } else if (mode != 4) {
                printf("Server mode is incorrect");
            } else {
                convertByteOrder(&recvMsg);
                recvMsg.recvTimestamp = time2ntp(mTime);

                printf("\nMessage received: \n");
                printf("\n");
                printMessageDetails(recvMsg, IP);
            }
        } else {
            printf("No replies from server. \n");
        }
    }
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

    msg->stratum = 1;
    msg->poll = 4;
    msg->precision = 4;
    msg->rootDelay = 0;
    msg->rootDispersion = 0; 

    msg->originateTimestamp = time2ntp(mTime);
    msg->transmitTimestamp = time2ntp(mTime);
}
