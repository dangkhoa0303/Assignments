#ifndef NTP_HELPER_H
#define NTP_HELPER_H

#include <netdb.h>

/* message format for sntp */
struct sntpMessageFormat {
  uint8_t li_vn_mode;
  uint8_t stratum;
  uint8_t poll;

  int8_t precision;
  int32_t rootDelay;

  uint32_t rootDispersion;
  uint32_t refIdentifier;
  uint64_t refTimestamp;
  uint64_t originateTimestamp;
  uint64_t recvTimestamp;
  uint64_t transmitTimestamp;
};

void initializeMessageFormat(struct sntpMessageFormat*);
void convertByteOrder(struct sntpMessageFormat*);
void printMessageDetails(struct sntpMessageFormat, char *);

#endif

  
