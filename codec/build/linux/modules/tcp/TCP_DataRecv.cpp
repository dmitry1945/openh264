#include "TCP_DataRecv.h"

void TCP_DataRecv::process_rx(void* buf, int n)
{
	// Here we process packet and decode them....
	uint8_t* data = (uint8_t*)buf;
	bool done = false;
	int start_pos = 0;
	int pos = 0;
	rxtx_packet* packet = (rxtx_packet*)this->packet_data;
	do
	{
		if (this->rx_state == rxtx_packet_state::PACKET_INIT)
		{
			for (pos = start_pos; pos < n; pos++)
			{
				uint32_t sync = 0;
				memcpy(&sync, &data[pos], sizeof(uint32_t));
				if (sync == RXTX_SYNC_VAL)
				{
					memcpy(this->packet_data, &data[pos], offsetof(rxtx_packet, data));
					//rxtx_packet* rx_pack = (rxtx_packet*)&data[pos];
					pos += offsetof(rxtx_packet, data);
					start_pos = pos;
					packet->rx_pos = 0;
					this->rx_state = rxtx_packet_state::PACKET_PROCESS;
					break;
				}
			}
		}
		if (this->rx_state == rxtx_packet_state::PACKET_PROCESS)
		{
			int copy_cnt = packet->length - packet->rx_pos;
			if (copy_cnt > (n - pos)) copy_cnt = n - pos;

			memcpy(&packet->data[packet->rx_pos], &data[pos], copy_cnt);
			//					this->rx_state = rxtx_packet_state::PACKET_INIT;
			pos += copy_cnt;
			packet->rx_pos += copy_cnt;
			if (packet->rx_pos == packet->length)
			{
				uint16_t crc = tcp_crc16(packet->data, packet->length);
				std::cout << "found sync pos = " << pos << ", packet->tx_count = " << packet->tx_count << ", len=" << packet->length << ", Crc_rx = " << packet->crc16 << ", crc_calc = " << crc << std::endl;
				if (this->cb != NULL)
				{
					this->cb(this->params, packet->data, packet->length);
				}
				this->rx_state = rxtx_packet_state::PACKET_INIT;
				start_pos = pos;
			}
		}

		if (pos >= n) done = true;
		
	} while (done == false);


	//std::cout << "process_rx = " << n << std::endl;
	//if (this->cb == NULL) return;
	//this->cb(this->params, buf, n);
}

void TCP_DataRecv::sign_callback(void* params, tcp_callback_ptr callback)
{
	this->params = params;
	this->cb = callback;
}

bool TCP_DataRecv::Send(void* data, int len, int id)
{
	return false;
}

