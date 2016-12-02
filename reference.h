#ifndef REFERENCE_H
#define REFERENCE_H

#include <netdb.h>
#include "ntp_helper.h"

uint64_t time2ntp (struct timeval);

struct timeval ntp2time (unsigned long long);

void printTime(struct timeval, char *, struct sntpMessageFormat);

#endif 
