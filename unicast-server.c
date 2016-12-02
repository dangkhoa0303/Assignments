#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include "u_server_helper.h"

#define PORT 1234 /* The server's listening port */

int main(int argc, char * argv[]) {
    int sockfd;
    struct sockaddr_in client_addr;

    /* open socket */
    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("error socket");
        exit(1);
    }
    /* zero struct */
    memset(&client_addr, 0, sizeof (client_addr));

    client_addr.sin_family = PF_INET;
    client_addr.sin_port = htons(PORT);
    client_addr.sin_addr.s_addr = INADDR_ANY;

    /* bind */
    if (bind(sockfd, (struct sockaddr *) &client_addr, sizeof(struct sockaddr)) == -1) {
        perror("error bind");
        exit(1);
    }
    printf("OK\n");

    /* infinite loop */
    while (1) {
        handle(sockfd, client_addr);
    }
    close(sockfd);

    return 0;
}
