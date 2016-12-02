#ifndef U_CLIENT_HELPER_H
#define U_CLIENT_HELPER_H

#include "ntp_helper.h"

void setMessage(struct sntpMessageFormat*, struct timeval);
void requestMessage(int, struct sockaddr_in, char *);

#endif
