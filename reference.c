#include "reference.h"
#include <stdio.h>
#include <time.h>

const unsigned long long OFFSET = 2208988800ULL;
const unsigned long long NTP_SCALE = 4294967295ULL;

/* convert from struct timeval to ntp time */
/* http://stackoverflow.com/questions/2641954/create-ntp-time-stamp-from-gettimeofday */
uint64_t time2ntp(struct timeval time) {
  unsigned long long time_ntp, time_usecs;

  time_ntp = time.tv_sec + OFFSET;
  time_usecs = (NTP_SCALE * time.tv_usec) / 1000000UL;

  return (time_ntp << 32) | time_usecs;
}

/* convert from ntp time to timeval */
struct timeval ntp2time(unsigned long long ntp) {
  unsigned long long time_secs, time_usecs;

  time_usecs = ntp & 0xFFFFFFFF;
  time_secs = (ntp >> 32) & 0xFFFFFFFF;
			
  struct timeval t;

  time_secs = time_secs - OFFSET;

  time_usecs = (time_usecs + 1) * 1000000UL / NTP_SCALE;

  t.tv_sec = (time_t) time_secs;
  t.tv_usec = (suseconds_t) time_usecs;

  return t;
}

/* print time from timeval struct */
/* Source: http://stackoverflow.com/questions/2408976/struct-timeval-to-printable-format */
void printTime(struct timeval tv, char *IP, struct sntpMessageFormat msg) {
  time_t nowtime;
  struct tm *nowtm;
  char tmbuf[64], buf[64];

  nowtime = tv.tv_sec;
  nowtm = localtime(&nowtime);

  strftime(tmbuf, sizeof(tmbuf), "%Y-%m-%d %H:%M:%S", nowtm);
  snprintf(buf, sizeof(buf), "%s.%06ld", tmbuf, tv.tv_usec);

  printf("%s ", buf);
  printf("%s s%d no-leap\n", IP, msg.stratum);
}
