#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <ctype.h>
#include <unistd.h>

#include "sfsource.h"
#include "serialpacket.h"
#include "serialprotocol.h"
#include "CBSweepDataMsg.h"
#include "CBRepoQueryMsg.h"
#include "CBChannelMaskMsg.h"

// c doesn't know about network types, but we want to include lqm.h
#define nx_struct struct
#define nxle_uint8_t uint8_t
#define nxle_int8_t int8_t
#define nxle_uint16_t uint16_t
#define nxle_uint32_t uint32_t
#define nx_uint8_t uint8_t
#define nx_int8_t int8_t
#define nx_uint16_t uint16_t
#define nx_uint32_t uint32_t
#include "spectrummonitor.h"

char *sfhost = NULL;
char *sfport = NULL;

void hexprint(uint8_t *packet, int len)
{
  int i;
  for (i = 0; i < len; i++)
    printf("0x%02x ", packet[i]);
}

void print_usage(int argc, char **argv)
{
  fprintf(stderr, "Usage: %s -h <host> -p <port>\n", argv[0]);
  fprintf(stderr, "Example: %s -h localhost -p 9002\n", argv[0]);
  // TODO
  
}

void parse_cmd_line_params(int argc, char **argv)
{
  int index;
  int c;

  opterr = 0;
  while ((c = getopt (argc, argv, "h:p:")) != -1)
    switch (c)
    {
      case 'h':
        sfhost = optarg;
        break;
      case 'p':
        sfport = optarg;
        break;
      default:
        print_usage(argc, argv);
        exit(1);
        break;
    }

  if (sfhost == NULL || sfport == NULL) {
    print_usage(argc, argv);
    exit(1);
  }
}

char convert_hexbyte(char *cptr) {
  // from: http://www.programmersheaven.com/download/3180/download.aspx
	char retval;
	char nbl;
	int shift;
	
	retval = 0;
	for( shift = 4; shift >= 0; shift -= 4 ) {
		if ((*cptr >= '0') && (*cptr <= '9')) {
			nbl = *cptr - '0';
		} else {
			if ((*cptr >= 'A') && (*cptr <= 'F')) {
				nbl = *cptr - 'A' + 10;		
      } else if ((*cptr >= 'a') && (*cptr <= 'f')) {
				nbl = *cptr - 'a' + 10;		
			} else {
        fprintf(stderr, "Packet content contains incorrect character %c\n", *cptr);
        exit(-1);
			}
		}
		++cptr;
		retval |= (nbl << shift);
	}
	return( retval );
}


int main(int argc, char **argv)
{
  int fd_sfsource;
  FILE *fp_tracefile; 
  int display = 1;

  parse_cmd_line_params(argc, argv);

  fd_sfsource = open_sf_source(sfhost, atoi(sfport));
  if (fd_sfsource < 0)
  {
    fprintf(stderr, "Couldn't connect to serial forwarder at %s:%s\n", sfhost, sfport);
    exit(1);
  }
  printf("# Listening to serial forwarder @ %s:%s\n", sfhost, sfport); 

  for (;;)
  {
    int len, i, k, sf_nodeID;
    char dateBuf[50], timeBuf[50];
    struct timeval tv;
    time_t curtime;
    uint8_t *packet = NULL;

    packet = read_sf_packet(fd_sfsource, &len); 

    if (!packet){
      printf("Error: invalid packet!\n"); 
      exit(0);
    }

/*    curtime=tv.tv_sec;*/
/*    strftime(dateBuf,50,"%H %M %S", localtime(&curtime));*/
/*    sprintf(timeBuf, "%s %ld", dateBuf, tv.tv_usec);       */

    if (len >= 1 + SPACKET_SIZE && packet[0] == SERIAL_TOS_SERIAL_ACTIVE_MESSAGE_ID)
    {
      tmsg_t *msg = new_tmsg(packet + 1 + SPACKET_SIZE, len - 1);
      uint8_t amtype = packet[SPACKET_SIZE];

      if (!msg){
        printf("Error: msg is NULL !!!\n");
        exit(0);
      }

      switch (amtype)
      {
        case AM_CB_SWEEP_DATA_MSG: printf("AM_CB_SWEEP_DATA_MSG\n"); break;
        case AM_CB_REPO_QUERY_MSG: printf("AM_CB_REPO_QUERY_MSG\n"); break;
        case AM_CB_CHANNELMASK_MSG: printf("AM_CB_CHANNELMASK_MSG\n"); break;
        default:
          printf("Unknown AM packet: ");
          hexprint(packet, len);
          break;
      }

      free(msg);
    }
    else
    {
      printf("non-AM packet: ");
      hexprint(packet, len);
    }
    fflush(stdout);
  }
}
