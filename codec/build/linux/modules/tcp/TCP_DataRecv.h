#pragma once

#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "stdint.h"
#include "rxtx_packet.h"

typedef void (*tcp_callback_ptr)(void* params, void* packet, int n);


class TCP_DataRecv
{
public:
	// Process received unmanaged buffer
	virtual void process_rx(void* buf, int n);
	virtual void sign_callback(void* params, tcp_callback_ptr callback);
	virtual bool Send(void* data, int len, int id = 0);

protected:
	void* params;
	tcp_callback_ptr cb;
	uint8_t packet_data[sizeof(rxtx_packet_t)];

	rxtx_packet_state rx_state;
};