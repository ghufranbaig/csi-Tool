/*
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>


#include <linux/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include<signal.h>

#include <tx80211.h>
#include <tx80211_packet.h>

#define DEST_MAC0	0x00
#define DEST_MAC1	0x00
#define DEST_MAC2	0x00
#define DEST_MAC3	0x00
#define DEST_MAC4	0x00
#define DEST_MAC5	0x00

#define ETHER_TYPE	0x003

#define DEFAULT_IF	"wlan0"
#define BUF_SIZ		110

struct tx80211	tx;
struct tx80211_packet	tx_packet;
uint8_t *payload_buffer;
#define PAYLOAD_SIZE	2000000

struct lorcon_packet
{
	//u_char	phyheader[21];
	__le16	fc;
	__le16	dur;
	u_char	addr1[6];
	u_char	addr2[6];
	u_char	addr3[6];
	__le16	seq;
	__le16  chan;
	__le16  seqNo;
	u_char	payload[0];
} __attribute__ ((packed));

struct rx_pkt
{
	u_char	phyheader[21];
	__le16	fc;
	__le16	dur;
	u_char	addr1[6];
	u_char	addr2[6];
	u_char	addr3[6];
	__le16	seq;
	__le16  chan;
	__le16  seqNo;
	u_char	payload[0];
} __attribute__ ((packed));


static void init_lorcon()
{
	/* Parameters for LORCON */
	int drivertype = tx80211_resolvecard("iwlwifi");

	/* Initialize LORCON tx struct */
	if (tx80211_init(&tx, "mon0", drivertype) < 0) {
		fprintf(stderr, "Error initializing LORCON: %s\n",
				tx80211_geterrstr(&tx));
		exit(1);
	}
	if (tx80211_open(&tx) < 0 ) {
		fprintf(stderr, "Error opening LORCON interface\n");
		exit(1);
	}

	/* Set up rate selection packet */
	tx80211_initpacket(&tx_packet);
}


static inline void payload_memcpy(uint8_t *dest, uint32_t length,
		uint32_t offset)
{
	uint32_t i;
	for (i = 0; i < length; ++i) {
		dest[i] = payload_buffer[(offset + i) % PAYLOAD_SIZE];
	}
}

static inline void make_pkt()
{
	

	struct lorcon_packet *packet;
	uint32_t packet_size = 100;

	/* Generate packet payloads */
	payload_buffer = malloc(packet_size);
	if (payload_buffer == NULL) {
		perror("malloc payload buffer");
		exit(1);
	}
	generate_payloads(payload_buffer, packet_size);
	/* Allocate packet */
	packet = malloc(sizeof(*packet) + packet_size);
	if (!packet) {
		perror("malloc packet");
		exit(1);
	}
	packet->fc = (0x08 /* Data frame */
				| (0x0 << 8) /* Not To-DS */);
	packet->dur = 0xffff;
	memcpy(packet->addr1, "\x00\x16\xea\x12\x34\x56", 6);
	memcpy(packet->addr2, "\x00\x16\xea\x12\x34\x56", 6);
	memcpy(packet->addr3, "\xff\xff\xff\xff\xff\xff", 6);

	packet->seq = 0;
	tx_packet.packet = (uint8_t *)packet;
	tx_packet.plen = sizeof(*packet) + packet_size;
	payload_memcpy(packet->payload, packet_size,
				0);

}

void send_pkt(uint16_t seq,uint16_t chan)
{
		struct lorcon_packet *ptr = tx_packet.packet;
		ptr->seqNo = seq;
		ptr->chan = chan;
		int ret = tx80211_txpacket(&tx, &tx_packet);
		if (ret < 0) {
			fprintf(stderr, "Unable to transmit packet: %s\n",
					tx.errstr);
			exit(1);
		}
		printf("sent pkt %u on channel %u\n",seq,chan);

}

void sig_handler(int signo){
	printf("killing\n");
	exit(1);
}

void set_channel(int chan) {
	char snum[10];
	snprintf(snum,10,"%i",chan);

	char command[100];
	command[0]='\0';	
	strcat(command,"iw mon0 set channel ");
	strcat(command,snum);
	strcat(command," HT20");

	printf("%s\n",command);

	system(command);

	command[0]='\0';	
	strcat(command,"iw wlan0 set channel ");
	strcat(command,snum);
	strcat(command," HT20");

	printf("%s\n",command);

	system(command);	

	printf("channel set to %i \n",chan);
}

int setup_socket(char* ifName) {

	int sockfd;
	//char ifName[IFNAMSIZ];
	int sockopt;
	struct ifreq ifopts;	/* set promiscuous mode */

	//strcpy(ifName, DEFAULT_IF);

	/* Open PF_PACKET socket, listening for EtherType ETHER_TYPE */
	if ((sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETHER_TYPE))) == -1) {
		perror("listener: socket");	
		return -1;
	}

	/* Set interface to promiscuous mode - do we need to do this every time? */
	strncpy(ifopts.ifr_name, ifName, IFNAMSIZ-1);
	ioctl(sockfd, SIOCGIFFLAGS, &ifopts);
	ifopts.ifr_flags |= IFF_PROMISC;
	ioctl(sockfd, SIOCSIFFLAGS, &ifopts);
	/* Allow the socket to be reused - incase connection is closed prematurely */
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt) == -1) {
		perror("setsockopt");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	/* Make recv non-blocking*/
	struct timeval read_timeout;
	read_timeout.tv_sec = 0;
	read_timeout.tv_usec = 10;
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof(read_timeout)) == -1)	{
		perror("SO_BINDTODEVICE");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	/* Bind to device */
	if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, ifName, IFNAMSIZ-1) == -1)	{
		perror("SO_BINDTODEVICE");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	return sockfd;

}

int main(int argc, char *argv[])
{
	int sockfd, ret;
	int sockopt;
	ssize_t numbytes;
	uint8_t buf[BUF_SIZ];
	char ifName[IFNAMSIZ];

	uint32_t mode=0;// 0: TX, 1: RX

	if (argc < 2 || (1 != sscanf(argv[1], "%u", &mode)))
		mode = 0;
	

	//signal(SIGKILL,sig_handler);
	//signal(SIGTERM,sig_handler);
	
	strcpy(ifName, DEFAULT_IF);

	/* Header structures */
	struct rx_pkt *eh = (struct rx_pkt *) buf;
	
	init_lorcon();
	make_pkt();

	struct timespec start, now;

	uint16_t channels[37]={1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161, 165};

	int i,j;
	int numPkt = 10;
	uint16_t curr_seq = 0;
	uint32_t interval = 10e6;
	int minPktsToRcv = 20;
	int32_t diff = 0;
	uint32_t index = 0;
	for (i=0;i<37;i++){
		set_channel(channels[i]);
		clock_gettime(CLOCK_MONOTONIC, &start);
		diff = 0;
		index = 0;
		int pkts_rcvd = 0;

		sockfd = setup_socket(ifName);
		while(diff < interval){
			if (mode == 0){
				for (j=0;j<numPkt;j++){
						send_pkt(index,channels[i]);
						index += 1;
				}
			} else {
				if (pkts_rcvd > minPktsToRcv) {
					send_pkt(0xFF,channels[i]);
				}
			}
			numbytes = recvfrom(sockfd, buf, BUF_SIZ, 0, NULL, NULL);
			if (numbytes > 25) {
				if (strncmp(eh->addr1,"\x00\x16\xea\x12\x34\x56",6) == 0 && strncmp(eh->addr2,"\x00\x16\xea\x12\x34\x56",6) == 0
				&& strncmp(eh->addr3,"\xff\xff\xff\xff\xff\xff",6) == 0){

						if (mode == 1) {
							pkts_rcvd += 1;
							printf ("%i,%i\n",i,eh->seqNo);
							recvfrom(sockfd, buf, BUF_SIZ, 0, NULL, NULL);

							clock_gettime(CLOCK_MONOTONIC, &start);

							//if (pkts_rcvd > minPktsToRcv) {
							//	break;
							//}
						} else {
							if (eh->seqNo == 0xFF){
								break;
							}
						}
				}
			}
			clock_gettime(CLOCK_MONOTONIC, &now);
			diff = (now.tv_sec - start.tv_sec) * 1000000 + (now.tv_nsec - start.tv_nsec) / 1000;

			if (mode == 1){
				if (diff > interval && pkts_rcvd > minPktsToRcv)
					break;
			}

			//printf("diff %i\n",diff);
		
		}

		close(sockfd);

	}

	printf("returning");
	close(sockfd);
	return ret;
}
