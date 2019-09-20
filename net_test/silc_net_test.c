/*
 * silc_net_test.c
 *
 *  Created on: Jul 15, 2012
 *      Author: jeff_zheng
 */

#include "silc_common.h"
#include "silc_net_test.h"
#include <popt.h>
#include <poll.h>

silc_net_test g_silc_net_test;

#define process_stats(a)   \
	uint64_t a ## _curr_sec = p_test->a - p_test->a ## _last_print; \
	p_test->a ##_last_print = p_test->a;\
	SILC_LOG("%-14s %12"PRIu64, #a, a ##_curr_sec);

#define process_statsk(a)   \
	uint64_t a ## _curr_sec = p_test->a - p_test->a ## _last_print; \
	p_test->a ##_last_print = p_test->a;\
	SILC_LOG("k%-13s %12"PRIu64, #a, (a ##_curr_sec)/1000);

void silc_net_test_server_print_stats(silc_net_test* p_test)
{
	if(p_test->arg.type == SILC_NET_TEST_SERVER)
	{
		process_stats(pkt_received);
		process_stats(bytes_received);
		process_stats(pkt_dropped);
		process_statsk(bits_received);
	}
	else
	{
		process_stats(pkt_sent);
		process_stats(bytes_sent);
		process_statsk(bits_sent);
		process_stats(tx_poll_cnt);
		process_stats(tx_egain_cnt);
	}
}

void* silc_net_test_prn_proc(void* p_arg)
{
	silc_net_test* p_test = p_arg;
	uint64_t ms_passed, ms_last_print = 0;
	uint64_t ms_start = silc_time_get_ms();
	while(1)
	{
		ms_passed = silc_time_get_ms() - ms_start;
		if(ms_passed - ms_last_print > 1000)
		{
			SILC_LOG("ms_passed %"PRIu64" ms_last_print %"PRIu64, ms_passed, ms_last_print);
			ms_last_print += 1000;
			silc_net_test_server_print_stats(p_test);
		}
		//sleep for 10ms
		silc_time_sleep(0, 10000000);
	}
}


int silc_net_test_init(silc_net_test_arg* p_arg)
{
	silc_net_test* p_test = &g_silc_net_test;

	memset(p_test, 0, sizeof(*p_test));
	p_test->arg = *p_arg;

	pthread_create(&p_test->prn_thread, NULL, silc_net_test_prn_proc, p_test);

	if(p_test->arg.mode == SILC_NET_TEST_UDP)		//server
		return silc_udp_test_init(p_test);
	else
		return silc_tcp_test_init(p_test);

	return 0;
}

int main(int argc, const char** argv)
{
	silc_net_test_arg arg =
	{
			.tx_msg_rate = 10000,
			.tx_msg_size = 64,
			.port = 12344,
			.tx_buf_size = 1024*1024,
			.rx_buf_size = 1024*1024,
	};
	char* test_mode = malloc(64);
	char* test_type = malloc(64);
	arg.server_addr = malloc(256);
	arg.bind_addr = malloc(256);

	arg.server_addr[0] = '\0';
	arg.bind_addr[0] = '\0';


	struct poptOption option_table[] =
	{
			{ "test-mode",        'm', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT, &test_mode,SILC_NET_TEST_OPT_TEST_MODE,
					"test mode, udp or tcp", NULL },
			{ "test-type",        'a', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT, &test_type,SILC_NET_TEST_OPT_TEST_TYPE,
					"test type, client or server", NULL },
			{ "port",'p', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT, &arg.port,SILC_NET_TEST_OPT_PORT,
					"port number", NULL },
			{ "server-addr",        's', POPT_ARG_STRING, &arg.server_addr,SILC_NET_TEST_OPT_SERVER_ADDR,
					"Address of the server, valid only when type is client", NULL },
			{ "bind-addr",        'c', POPT_ARG_STRING, &arg.bind_addr,SILC_NET_TEST_OPT_BIND_ADDR,
					"Address to bind, valid only when type is server", NULL },
			{ "tx-pkt-size", 't', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT, &arg.tx_msg_size,SILC_NET_TEST_OPT_TX_PKT_SIZE,
					"size of packet to transmit", NULL },
			{ "tx-pkt-rate", 'T', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT, &arg.tx_msg_rate,SILC_NET_TEST_OPT_TX_PKT_RATE,
					"Target packet transmit rate", NULL },
			{ "tx-buf-size", 'b', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT, &arg.tx_buf_size,SILC_NET_TEST_OPT_TX_BUF_SIZE,
					"Transmit socket buffer size", NULL },
			{ "rx-buf-size", 'B', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT, &arg.rx_buf_size,SILC_NET_TEST_OPT_RX_BUF_SIZE,
					"Receive socket buffer size", NULL },		POPT_AUTOHELP

		POPT_TABLEEND
	};
	poptContext option_context = poptGetContext(argv[0], argc, argv, option_table, 0);

	if (argc < 2)
	{
		poptPrintUsage(option_context, stderr, 0);
		exit(-1);
	}

	int code;
	while ((code = poptGetNextOpt(option_context)) >= 0)
	{
		switch (code)
		{
		case SILC_NET_TEST_OPT_TEST_MODE:
			if(test_mode)
			{
				if(strcmp(test_mode, "udp")==0)
					arg.mode = SILC_NET_TEST_UDP;
				else if(strcmp(test_mode, "tcp")==0)
					arg.mode = SILC_NET_TEST_TCP;
				else
				{
					SILC_ERR("Invalid test mode %s", test_mode);
					exit(1);
				}
			}
			else
			{
				SILC_ERR("Must specify test mode.");
				exit(1);
			}
			break;
		case SILC_NET_TEST_OPT_TEST_TYPE:
			if(test_type)
			{
				if(strcmp(test_type, "client")==0)
					arg.type = SILC_NET_TEST_CLIENT;
				else if(strcmp(test_type, "server")==0)
					arg.type = SILC_NET_TEST_SERVER;
				else
				{
					SILC_ERR("Invalid test type %s", test_type);
					exit(1);
				}
			}
			else
			{
				SILC_ERR("Must specify test type.");
				exit(1);
			}
			break;
		case SILC_NET_TEST_OPT_PORT:
			if(arg.port>=65535 || arg.port <=0)
			{
				SILC_ERR("Invalid port specified %d", arg.port);
				exit(1);
			}
			break;
		case SILC_NET_TEST_OPT_SERVER_ADDR:
			break;
		case SILC_NET_TEST_OPT_BIND_ADDR:
			break;
		case SILC_NET_TEST_OPT_TX_PKT_SIZE:
			if(arg.tx_msg_size>= (65535 - 20 -14 - 16) || arg.tx_msg_size <=0)
			{
				SILC_ERR("Invalid tx packet size specified %d", arg.tx_msg_size);
				exit(1);
			}
			break;
		case SILC_NET_TEST_OPT_TX_PKT_RATE:
			if(arg.tx_msg_rate<=0)
			{
				SILC_ERR("Invalid tx packet rate specified %d", arg.tx_msg_rate);
				exit(1);
			}
			break;
		}
	}

	if(arg.bind_addr)
	{
		if(strcmp(arg.bind_addr, "") == 0)
		{
			free(arg.bind_addr);
			arg.bind_addr = NULL;
		}
	}
	silc_time_lib_init();

	silc_net_test_init(&arg);


	return 0;
}
