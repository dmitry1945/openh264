#pragma once
#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <stdint.h>
#include <stddef.h>

#ifndef MAX_RXTX_LENGTH
#define MAX_RXTX_LENGTH		(65536*4 - 14)
#endif // MAX_RXTX_LENGTH

#define RXTX_SYNC_VAL	0x12345678

enum rxtx_packet_state
{
	PACKET_INIT,
	PACKET_DONE,
	PACKET_PROCESS
};

typedef struct rxtx_packet
{
	uint32_t	sync;
	uint32_t	length;
	uint32_t	tx_count;
	unsigned short	crc16;
	unsigned short	allign;
	uint8_t		data[MAX_RXTX_LENGTH];
	uint32_t    rx_pos;
	static void fill(uint8_t* buff, int len, rxtx_packet* packet);
}rxtx_packet_t;

unsigned short tcp_crc16(const unsigned char* data_p, int length);
