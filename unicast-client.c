#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include "u_client_helper.h"

#define PORT 1234
#define SLEEP_TIME 5

int main (int argc, char * argv[]) {
    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent *he;
    char uweIP[] = "ntp.uwe.ac.uk";
    char *serverIP;

    if (argc == 1) { // if no argument for IP address, use uwe as default
        serverIP = malloc(sizeof(uweIP));
        strcpy(serverIP, uweIP);

    } else if (argc == 2) { // get argument of IP address
        serverIP = malloc(sizeof(argv[1]));
        strcpy(serverIP, argv[1]);
    }
    printf("OK\n");
    printf("%s\n",serverIP);

    if ((he = gethostbyname(serverIP)) == NULL) {
        perror("Server gethostbyname");
        exit(1);
    }

    /* open socket */
    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket error");
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

    memset(&server_addr, 0, sizeof (server_addr));
    server_addr.sin_family = PF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr = *((struct in_addr *) he->h_addr);
    server_addr.sin_addr.s_addr = inet_addr(serverIP);
    
    while(1) {
        requestMessage(sockfd, server_addr, serverIP);
        sleep(SLEEP_TIME);
    }
    
    close(sockfd);

    return 0;
}
