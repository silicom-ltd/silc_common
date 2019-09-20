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

int silc_tcp_test_send_packet(int sock, char* buf, int size)
{
    int ret;
//    silc_sock_addr_dump(p_test->p_remote_addr);
    ret = send(sock, buf, size, MSG_DONTWAIT);
    return ret;

}

int silc_tcp_test_receive_packet(int sock, char* buf, int buf_size)
{
	int ret;
	ret = recv(sock, buf, buf_size, 0);
	return ret;
}

int silc_tcp_test_server_process_pkt(silc_net_test* p_test)
{
	p_test->bytes_received += p_test->recv_pkt_size;
	p_test->bits_received += (p_test->recv_pkt_size<<3);
	return 0;
}

void* silc_tcp_test_client_proc(void* p_arg)
{
	silc_net_test* p_test = p_arg;
	uint64_t curr_loop, curr_limit;
	uint64_t ms_passed;
	uint64_t pkt_est_total;
	uint64_t pkt_sent = 0;
	int ret;

	struct pollfd  pollfds;
	uint64_t ms_start = silc_time_get_ms();

	SILC_LOG("Connect to: %s", inet_ntoa(((struct sockaddr_in*)p_test->p_remote_addr)->sin_addr));
	if(silc_sock_client_connect(p_test->sock, p_test->p_remote_addr, p_test->remote_len) != 0)
	{
		SILC_ERR("Failed to connect server");
		close(p_test->sock);
		exit(1);
	}

	while(1)
	{
		curr_loop = 0;
		ms_passed = silc_time_get_ms() - ms_start;
		pkt_est_total = ms_passed * p_test->arg.tx_msg_rate / 1000;
		curr_limit = pkt_est_total - pkt_sent;
		for(curr_loop=0; curr_loop < curr_limit;)
		{
			ret = silc_tcp_test_send_packet(p_test->sock, p_test->send_buf, p_test->arg.tx_msg_size);
			if(ret > 0)
			{
				p_test->pkt_sent++;
				p_test->bytes_sent += p_test->arg.tx_msg_size;
				p_test->bits_sent += (p_test->arg.tx_msg_size<<3);
				curr_loop ++;
			}
			else
			{
				if(errno != EAGAIN)
				{
					SILC_ERR("Failed to send tcp packet error:%d:%s", errno, strerror(errno));
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

void silc_tcp_test_server_receive_proc(silc_net_test* p_test, int recv_fd)
{
	int ret = -1, loop, loop_max = 1000;

	for(loop = 0; loop < loop_max ; loop++)
	{
		ret = silc_tcp_test_receive_packet(recv_fd, p_test->recv_buf, sizeof(p_test->recv_buf));
		if(ret > 0)
		{
			p_test->recv_pkt_size = ret;
			silc_tcp_test_server_process_pkt(p_test);
		}
		else
			break;
	}
}

int silc_tcp_test_server_proc(silc_net_test* p_test)
{
	int ret = -1;
	int accept_fd, addr_len;
	struct sockaddr_in accept_addr;
	int i, pollcnt = 1;
	struct pollfd pollfds[SILC_NET_TEST_CLIENT_MAX];
	struct pollfd pollcur[SILC_NET_TEST_CLIENT_MAX];
	pollfds[0].fd = p_test->sock;
	pollfds[0].events = POLLIN;
	pollfds[0].revents = 0;

	if(silc_sock_server_listen(p_test->sock, SILC_NET_TEST_CLIENT_MAX) != 0)
	{
		SILC_ERR("Failed listen");
		close(p_test->sock);
		exit(1);
	}

	while(1)
	{
		memcpy(pollcur, pollfds, sizeof(struct pollfd)*pollcnt);
		ret = poll(&pollcur[0], pollcnt, 1000);
		if(ret == 0)
			continue;
		else
		{
			if(ret <0)
			{
				SILC_ERR("poll packet error %d:%s", errno, strerror(errno));
			}
		}

		if(pollcur[0].revents & POLLIN)
		{
			accept_fd = silc_sock_server_accept(pollcur[0].fd,
					(struct sockaddr*)&accept_addr, (socklen_t*)&addr_len);
			if(accept_fd > 0)
			{
				pollfds[pollcnt].fd = accept_fd;
				pollfds[pollcnt].events = POLLIN;
				pollfds[pollcnt].revents = 0;
				pollcnt++;
				SILC_LOG("Accept connect from: %s", inet_ntoa(accept_addr.sin_addr));
			}
		}

		for(i=1; i<pollcnt; i++)
		{
			if(pollcur[i].revents & POLLIN)
			{
				silc_tcp_test_server_receive_proc(p_test, pollcur[i].fd);
			}
		}
	}
	return 0;
}

int silc_tcp_test_init(silc_net_test* p_test)
{
	if(p_test->arg.type == SILC_NET_TEST_SERVER)		//server
	{
		p_test->sock = silc_sock_server_open(AF_INET, SOCK_STREAM, p_test->arg.bind_addr, p_test->arg.port, p_test->arg.rx_buf_size, p_test->arg.tx_buf_size);
		if(p_test->sock <0)
			return -1;
		silc_tcp_test_server_proc(p_test);
	}
	else
	{
		p_test->sock = silc_sock_server_open(AF_INET, SOCK_STREAM, NULL, p_test->arg.port, p_test->arg.rx_buf_size, p_test->arg.tx_buf_size);
		p_test->remote_len = silc_sock_addr_prepare(AF_INET, p_test->arg.server_addr, p_test->arg.port,
				(struct sockaddr*)&p_test->sockaddr_store, sizeof(p_test->sockaddr_store));
		p_test->p_remote_addr = (struct sockaddr*)&p_test->sockaddr_store;
		if(0!=pthread_create(&p_test->client_thread, NULL, silc_tcp_test_client_proc, p_test))
		{
			SILC_ERR("Failed to start client thread, err:%s", strerror(errno));
			return -1;
		}
		while(1)
			sleep(1);
	}

	return 0;

}

