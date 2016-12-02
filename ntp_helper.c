#include <stdio.h>
#include <math.h>
#include <time.h>
#include "ntp_helper.h"
#include "reference.h"

/* initialize all fields to be 0 */
void initializeMessageFormat(struct sntpMessageFormat *format) {
  format->li_vn_mode = 0;
  format->stratum = 0;
  format->poll = 0;
  format->precision = 0;
  format->rootDelay = 0;
  format->rootDispersion = 0;
  format->refIdentifier = 0;
  format->refTimestamp = 0;
  format->originateTimestamp = 0;
  format->recvTimestamp = 0;
  format->transmitTimestamp = 0;
}

/* convert message format from hots byte order to network byte order and vice versa */
void convertByteOrder(struct sntpMessageFormat *msg) {
  /* htobenn -- host to big endian (32 or 64) */
  msg->rootDelay = htobe32(msg->rootDelay);
  msg->rootDispersion = htobe32(msg->rootDispersion);
  msg->refIdentifier = htobe32(msg->refIdentifier);
  msg->refTimestamp = htobe64(msg->refTimestamp);
  msg->originateTimestamp = htobe64(msg->originateTimestamp);
  msg->recvTimestamp = htobe64(msg->recvTimestamp);
  msg->transmitTimestamp = htobe64(msg->transmitTimestamp);
}

/* print out message  details */
void printMessageDetails(struct sntpMessageFormat msg, char *IP) {
  printf("Leap Indicator: %d\n", msg.li_vn_mode >> 6);
  printf("Version Number: %d\n", (msg.li_vn_mode >> 3) & 0x07);
  printf("Mode: %d\n", msg.li_vn_mode & 0x07);

  if (msg.stratum == 1) {
     printf("Peer Clock Stratum: primary reference (%d)\n", msg.stratum);
  } else if (msg.stratum == 0) {
     printf("Peer Clock Stratum: kiss-o'-death message (%d)\n", msg.stratum);
  }

  printf("Peer Polling Interval: %d (%f sec)\n", msg.poll, pow(2, msg.poll));
  printf("Peer Clock Precision: %f sec\n", pow(2, msg.precision));

  printf("Root Delay: %d sec\n", msg.rootDelay);
  printf("Root Dispersion: %d sec\n", msg.rootDispersion);

  /* get reference identifier char into a char array because refTimestamp is a bitstring containing code */
  /* shift right 8 bits in each iteration to get the corresponding code of character, and store in refCode */
  /* refCode is the actual code of refIdentifier */
  char refCode[5];
  int i;
  uint32_t temp = msg.refIdentifier;

  for (i=3; i>=0; i--) {
    refCode[i] = temp & 0xFF;
    temp = temp >> 8;
  }

  refCode[4] = '\0';
  printf("Reference Identifier: %s\n", refCode);

  printf("Reference Timestamp: ");
  printTime(ntp2time(msg.refTimestamp), IP, msg);

  printf("Originate Timestamp: ");
  printTime(ntp2time(msg.originateTimestamp), IP, msg);

  printf("Receive Timestamp: ");
  printTime(ntp2time(msg.recvTimestamp), IP, msg);

  printf("Transmit Timestamp: ");
  printTime(ntp2time(msg.transmitTimestamp), IP, msg);
}
