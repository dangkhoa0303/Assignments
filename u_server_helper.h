#ifndef U_SERVER_HELPER_H
#define U_SERVER_HELPER_H

#include "ntp_helper.h"

void setMessage(struct sntpMessageFormat*, struct timeval);
void handle(int, struct sockaddr_in);

#endif
