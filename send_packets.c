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
#define BUF_SIZ		2048

struct tx80211	tx;
struct tx80211_packet	tx_packet;
uint8_t *payload_buffer;
#define PAYLOAD_SIZE	2000000

struct lorcon_packet
{
	__le16	fc;
	__le16	dur;
	u_char	addr1[6];
	u_char	addr2[6];
	u_char	addr3[6];
	__le16	seq;
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

static inline void make_ack()
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

void send_pkt(int seq)
{
		struct lorcon_packet *ptr = tx_packet.packet;
		ptr->seq = seq;
		
		int ret = tx80211_txpacket(&tx, &tx_packet);
		if (ret < 0) {
			fprintf(stderr, "Unable to transmit packet: %s\n",
					tx.errstr);
			exit(1);
		}

}


int main(int argc, char *argv[])
{
	init_lorcon();
	make_ack();

	int i = 0;
	while(1){
		send_pkt(i);
		printf("packet %i sent\n",i);
		i+=1;
	}

	return 1;
}
