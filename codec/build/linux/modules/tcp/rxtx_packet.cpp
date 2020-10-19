#include "rxtx_packet.h"

unsigned short tcp_crc16(const unsigned char* data_p, int length)
{
    unsigned char x;
    unsigned short crc = 0xFFFF;

    while (length--) {
        x = crc >> 8 ^ *data_p++;
        x ^= x >> 4;
        crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x << 5)) ^ ((unsigned short)x);
    }
    return crc;
}

int tx_count_common = 0;
void rxtx_packet::fill(uint8_t* buff, int len, rxtx_packet* packet)
{
	packet->sync = RXTX_SYNC_VAL;
	packet->length = len;
	memcpy(packet->data, buff, packet->length);
    //packet->data[0] = 0x33;
    //packet->data[1] = 0x22;
    packet->crc16 = tcp_crc16(packet->data, len);
    packet->tx_count = tx_count_common++;
}

