/*
 * silc_net_test.c
 *
 *  Created on: Jul 15, 2012
 *      Author: jeff_zheng
 */

#include "silc_common.h"
#include <popt.h>
#include <poll.h>

#ifndef SILC_NET_TEST_
#define SILC_NET_TEST_

#define SILC_NET_TEST_CLIENT_MAX		10

typedef struct silc_net_test_hdr_s
{
	uint32_t seq;
}silc_net_test_hdr;
typedef enum silc_net_test_type_e
{
	SILC_NET_TEST_SERVER,
	SILC_NET_TEST_CLIENT,
}silc_net_test_type;
typedef enum silc_net_test_mode_e
{
	SILC_NET_TEST_UDP,
	SILC_NET_TEST_TCP,
}silc_net_test_mode;

typedef struct silc_net_test_arg_s
{
	silc_net_test_mode mode;	//0 for udp, 1 for tcp
	silc_net_test_type type;	//0 for server, 1 for client
	int tx_msg_rate;
	int tx_msg_size;
	int port;
	char* server_addr;	//valid for server
	char* bind_addr;		//valid for client
	int tx_buf_size;
	int rx_buf_size;
}silc_net_test_arg;

typedef struct silc_net_test_s
{
	silc_net_test_arg arg;
	int sock;
	struct sockaddr_storage sockaddr_store;
	struct sockaddr*		p_remote_addr;
	socklen_t				remote_len;
	int						recv_pkt_size;

	char					recv_buf[65536];
	char					send_buf[65536];
	silc_net_test_hdr*		p_hdr;

	pthread_t				client_thread;
	pthread_t				prn_thread;

	uint64_t				bytes_sent;
	uint64_t				bytes_sent_last_print;
	uint64_t				bits_sent;
	uint64_t				bits_sent_last_print;
	uint64_t				pkt_sent;
	uint64_t				pkt_sent_last_print;
	uint64_t				pkt_received;
	uint64_t				pkt_received_last_print;
	uint64_t				bytes_received;
	uint64_t				bytes_received_last_print;
	uint64_t				bits_received;
	uint64_t				bits_received_last_print;
	uint64_t				pkt_dropped;
	uint64_t				pkt_dropped_last_print;
	uint32_t				pkt_last_seq;
	uint64_t				tx_poll_cnt;
	uint64_t				tx_poll_cnt_last_print;
	uint64_t				tx_egain_cnt;
	uint64_t				tx_egain_cnt_last_print;
}silc_net_test;

typedef enum silc_net_test_opt_e{
	SILC_NET_TEST_OPT_TEST_MODE = 1,
	SILC_NET_TEST_OPT_TEST_TYPE,
	SILC_NET_TEST_OPT_PORT,
	SILC_NET_TEST_OPT_SERVER_ADDR,
	SILC_NET_TEST_OPT_BIND_ADDR,
	SILC_NET_TEST_OPT_TX_PKT_SIZE,
	SILC_NET_TEST_OPT_TX_PKT_RATE,
	SILC_NET_TEST_OPT_RX_BUF_SIZE,
	SILC_NET_TEST_OPT_TX_BUF_SIZE,

}silc_net_test_opt;

int silc_udp_test_init(silc_net_test* p_test);
int silc_tcp_test_init(silc_net_test* p_test);

#endif /*SILC_NET_TEST_*/
