/*
This program is now able to listen and send data to the mote simultaneosuly. pthreads have been used so that now listen() is non blocking.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>

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
#define NUM_THREADS 1
#include "spectrummonitor.h"


char *sfhost;
char *sfport;

struct arg_struct {
    int arg1;
    char* arg2;
    char* arg3;
};


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

int open_connection(int argc, char **argv)
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
	return fd_sfsource;
}

int listen_message(void* struct1)
{  
  int fd_sfsource;
  char *sfhost;
  char *sfport;
  struct arg_struct *args = struct1;
  fd_sfsource = args->arg1;
  sfhost = args->arg2;
  sfport = args->arg3;
  if(sfhost==NULL || sfport==NULL)
  {
	printf("First open a connection before listening\n");
	exit(1);
  }
  printf("# Listening to serial forwarder @ %s:%s\n", sfhost, sfport); 

  for (;;)
  {
    int len, i, k, sf_nodeID;
    char dateBuf[50], timeBuf[50];
    struct timeval tv;
    time_t curtime;
    uint8_t *packet=NULL;

    packet = read_sf_packet(fd_sfsource, &len); 

    if (!packet){
      printf("Error: invalid packet!\n"); 
      exit(0);
    }
    uint8_t amtype = packet[SPACKET_SIZE];

/*    curtime=tv.tv_sec;*/
/*    strftime(dateBuf,50,"%H %M %S", localtime(&curtime));*/
/*    sprintf(timeBuf, "%s %ld", dateBuf, tv.tv_usec);       */

    if (len >= 1 + SPACKET_SIZE && packet[0] == SERIAL_TOS_SERIAL_ACTIVE_MESSAGE_ID)
    {
      tmsg_t *msg = new_tmsg(packet + 1, len - 1);
      uint8_t amtype = packet[SPACKET_SIZE];

      if (!msg){
        printf("Error: msg is NULL !!!\n");
        exit(0);
      }
      printf("dest %u, src %u, length %u, group %u, type %u\n  ",
              spacket_header_dest_get(msg),
              spacket_header_src_get(msg),
              spacket_header_length_get(msg),
              spacket_header_group_get(msg),
              spacket_header_type_get(msg));
      hexprint((uint8_t *)tmsg_data(msg) + spacket_data_offset(0), tmsg_length(msg) - spacket_data_offset(0));

      switch (amtype)
      {
        case AM_CB_SWEEP_DATA_MSG: printf("\nAM_CB_SWEEP_DATA_MSG\n"); break;
        case AM_CB_REPO_QUERY_MSG: printf("\nAM_CB_REPO_QUERY_MSG\n"); break;
        case AM_CB_CHANNELMASK_MSG: printf("\nAM_CB_CHANNELMASK_MSG\n"); break;
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
// pthread_exit(NULL);
}

int send_message(int fd_sfsource, char *sfhost, char *sfport,  int data, int count)
{

  int returnval;
//  int i;
  if(sfhost==NULL || sfport==NULL)
  {
        printf("First open a connection before sending\n");
        exit(1);
  }
//This Part is Commented
/*  unsigned char *packet;
  packet = malloc(count);
  if (!packet)
    exit(2);

  for (i = 0; i < count; i++)
    packet[i] = data;

  fprintf(stderr,"Sending ");
  for (i = 0; i < count; i++)
    fprintf(stderr, " %02x", packet[i]);
  fprintf(stderr, "\n");

  returnval = write_sf_packet(fd_sfsource, packet, count);*/
//Till here

    uint16_t moteId;
    uint8_t i;
    uint8_t packet_size = 1+ SPACKET_SIZE;
    unsigned char *packet;
    tmsg_t* msg;

    packet = malloc(packet_size);
   
        if (!packet)
            exit(2);

    for(i=0;i<packet_size;i++)
        packet[i] = 0;

//  packet[0] == CB_CHANNELMASK_MSG_AM_TYPE;

    msg = new_tmsg(packet + 1,packet_size - 1);

//    printf("Input mote destination address: ");
//    scanf("%hu",&moteId);
    spacket_header_dest_set(msg,65535);
    spacket_header_src_set(msg, 0);
    spacket_header_length_set(msg, CB_CHANNELMASK_MSG_DATA_SIZEBITS);
    spacket_header_type_set(msg, CB_CHANNELMASK_MSG_AM_TYPE);

    returnval = write_sf_packet(fd_sfsource, packet, packet_size);
    printf("Sent \n");
  return returnval;
}


int main(int argc, char **argv)
{
	int fd,count;
	int data;
	int rc;
   	long t;
	char c;
        pthread_t thread1;
	fd = open_connection(argc, argv);
	struct arg_struct struct1;
	struct1.arg1 = fd;
	struct1.arg2 = sfhost;
	struct1.arg3 = sfport;
//      for (t=0;t<NUM_THREADS;t++)
//	{
	printf("In main: creating thread %ld\n", t);	
	rc = pthread_create(&thread1, NULL,(void*) listen_message,(void*)&struct1);
        if (rc){
          printf("ERROR; return code from pthread_create() is %d\n", rc);
          exit(-1);
	}
//	}
	data = 0;
//	printf("Enter the data to be sent\n");
//	scanf("%d",&data);
	send_message(fd, sfhost, sfport, data, 1);
   	return pthread_join(thread1, NULL); /* Wait until thread is finished */
	return 0;
} 
