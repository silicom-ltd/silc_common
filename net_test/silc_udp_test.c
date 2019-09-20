/*
 * silc_udp_test.c
 *
 *  Created on: Jul 15, 2012
 *      Author: jeff_zheng
 */

#include "silc_common.h"
#include "silc_common/silc_net.h"
#include "silc_net_test.h"
#include <popt.h>
#include <poll.h>

int silc_udp_test_send_packet(silc_net_test* p_test, char* buf, int size)
{
    int ret;
//    silc_sock_addr_dump(p_test->p_remote_addr);
    ret = sendto(p_test->sock, buf, size, MSG_DONTWAIT,  p_test->p_remote_addr,  p_test->remote_len);
    return ret;

}


int silc_udp_test_receive_packet(silc_net_test* p_test, char* buf, int buf_size)
{
	int ret;
	p_test->remote_len = sizeof(p_test->sockaddr_store);
	ret = recvfrom(p_test->sock, buf, buf_size, 0, p_test->p_remote_addr, &p_test->remote_len);
	return ret;
}

int silc_udp_test_wait_packet(silc_net_test* p_test)
{
	int ret;

	struct pollfd pfd = {p_test->sock, POLLIN, 0 };

//	uint64_t ms_end;
//	uint64_t ms_start = silc_time_get_ms();
	ret = poll(&pfd, 1, 1000);
//	ms_end = silc_time_get_ms();
//	SILC_LOG("poll return on %"PRIu64" ms", ms_end - ms_start);

	return ret;
}

int silc_udp_test_server_process_pkt(silc_net_test* p_test)
{
	silc_net_test_hdr* p_hdr = (silc_net_test_hdr*)(p_test->recv_buf);

	uint32_t new_seq = ntohl(p_hdr->seq);
	uint32_t seq_gap = new_seq - p_test->pkt_last_seq;
	if(seq_gap < 0x1000000U)
		p_test->pkt_dropped += (seq_gap - 1);
	else
		SILC_LOG("seq wrapped, old %10u, new %10u", p_test->pkt_last_seq, new_seq);

	p_test->pkt_last_seq = new_seq;
	p_test->pkt_received ++;
	p_test->bytes_received += p_test->recv_pkt_size;
	p_test->bits_received += (p_test->recv_pkt_size<<3);
	return 0;
}

void* silc_udp_test_client_proc(void* p_arg)
{
	silc_net_test* p_test = p_arg;
	uint64_t curr_loop, curr_limit;
	uint64_t ms_passed;
	uint64_t pkt_est_total;
	uint64_t pkt_sent = 0;
	int ret;
	silc_net_test_hdr* p_hdr;

	struct pollfd  pollfds;

	uint64_t ms_start = silc_time_get_ms();
	while(1)
	{
		curr_loop = 0;
		ms_passed = silc_time_get_ms() - ms_start;
		pkt_est_total = ms_passed * p_test->arg.tx_msg_rate / 1000;
		curr_limit = pkt_est_total - pkt_sent;
		for(curr_loop=0; curr_loop < curr_limit;)
		{
			p_hdr = (silc_net_test_hdr*)(p_test->send_buf);
			p_hdr->seq = htonl((uint32_t)(pkt_sent +1));

			ret = silc_udp_test_send_packet(p_test, p_test->send_buf, p_test->arg.tx_msg_size);
			if(ret > 0)
			{
				p_test->pkt_sent ++;
				p_test->bytes_sent += p_test->arg.tx_msg_size;
				p_test->bits_sent += (p_test->arg.tx_msg_size<<3);
				curr_loop ++;
			}
			else
			{
				if(errno != EAGAIN)
				{
					SILC_ERR("Failed to send udp packet error:%d:%s", errno, strerror(errno));
				}
				else
				{
					p_test->tx_egain_cnt ++;
				}
				break;
			}
			pkt_sent ++;
		}
		while(1)
		{
			p_test->tx_poll_cnt ++;
			memset(&pollfds, 0, sizeof(pollfds));
			pollfds.fd = p_test->sock;
			pollfds.events = POLLOUT;
			ret = poll(&pollfds, 1, 1000);
			if(ret>0)
				break;
		}
	}

	return NULL;

}

int silc_udp_test_server_proc(silc_net_test* p_test)
{
	int ret = -1, loop, loop_max = 1000;


	while(1)
	{

		ret = silc_udp_test_wait_packet(p_test);
		if(ret == 0)
			continue;
		else
		{
			if(ret <0)
			{
				SILC_ERR("poll packet error %d:%s", errno, strerror(errno));
			}
		}
		for(loop = 0; loop < loop_max ; loop++)
		{
			ret = silc_udp_test_receive_packet(p_test, p_test->recv_buf, sizeof(p_test->recv_buf));
			if(ret > 0)
			{
				p_test->recv_pkt_size = ret;
				silc_udp_test_server_process_pkt(p_test);
			}
			else
				break;
		}
	}
	return 0;
}

int silc_udp_test_init(silc_net_test* p_test)
{
	if(p_test->arg.type == SILC_NET_TEST_SERVER)		//server
	{
		p_test->sock = silc_sock_server_open(AF_INET, SOCK_DGRAM, p_test->arg.bind_addr, p_test->arg.port, p_test->arg.rx_buf_size, p_test->arg.tx_buf_size);
		if(p_test->sock <0)
			return -1;
	}
	else
	{
		p_test->sock = silc_sock_server_open(AF_INET, SOCK_DGRAM, NULL, p_test->arg.port, p_test->arg.rx_buf_size, p_test->arg.tx_buf_size);
		p_test->remote_len = silc_sock_addr_prepare(AF_INET, p_test->arg.server_addr, p_test->arg.port,
				(struct sockaddr*)&p_test->sockaddr_store, sizeof(p_test->sockaddr_store));
		p_test->p_remote_addr = (struct sockaddr*)&p_test->sockaddr_store;

		if(0!=pthread_create(&p_test->client_thread, NULL, silc_udp_test_client_proc, p_test))
		{
			SILC_ERR("Failed to start client thread, err:%s", strerror(errno));
			return -1;
		}
	}

	silc_udp_test_server_proc(p_test);

	return 0;

}

