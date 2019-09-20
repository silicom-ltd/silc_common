/*
 * silc_test_socket_lib.c
 *
 *  Created on: Apr 13, 2011
 *      Author: jack_chen
 */

#include "silc_test_socket_lib.h"

#define SILC_TEST_SOCKET_OPT_O(ARG_TYPE,NAME,ATTR,VALUE) \
	if(is_##NAME)  \
	{ \
		if(VALUE != 0) \
		{ \
			s_opt.o.NAME.ATTR = silc_true; \
		} \
		else \
		{ \
			s_opt.o.NAME.ATTR = silc_false; \
		} \
		printf("%s %s %s is %d\n",#ARG_TYPE,#NAME,#ATTR,s_opt.o.NAME.ATTR); \
	} \
	else \
	{ \
		printf("if set %s,test %s only\n",#ATTR,#NAME);  \
		return; \
	} \

#define SILC_TEST_SOCKET_BUF_LEN 1024


typedef void (* process_socket)(silc_socket_opt* opt,uint32_t create_recv_rng);

/**
 * @ingroup silc_socket
 * @brief call back for new connection
 *
 * @param[in] p_socket	the socket on which the connection is received
 * @param[in] p_usr_arg the p_usr_arg passed to silc_socket_create
 * @param[in] p_conn    the new connection
 * @return 0 to accept the connection. other value to reject the connection
 *
 * the infrastructure will call this callback to notify the user of a new connection.
 * The user may choose to accept by returning 0 and do any initialization in this function
 * or return other values to reject the connection
 **/
int silc_test_socket_callback_conn_new(silc_socket* p_socket, void* p_svr_arg, silc_sock_conn_id p_conn)
{
	printf("silc_test_socket_callback_conn_new\n");
	return 0;
}

/**
 * @ingroup silc_socket
 * @brief call back for close of a connection
 *
 * @param[in] p_socket	the socket on which the connection is received
 * @param[in] p_usr_arg the p_usr_arg passed to silc_socket_create
 * @param[in] p_conn    the new connection
 * @param[in] p_conn_arg    user argument of this conneciton
 * @return 0 to accept the connection. other value to reject the connection
 *
 * the infrastructure will call this callback to notify the user of the closing of a connection.
 * user should free any relevant resources.
 **/
int silc_test_socket_callback_conn_close(silc_socket* p_socket, void* p_svr_arg, silc_sock_conn_id p_conn, void* p_conn_arg)
{
	printf("silc_test_socket_callback_conn_close\n");
	return 0;
}

/**
 * @ingroup silc_socket
 * @brief call back for data receive
 *
 * @param[in] p_socket	the socket on which the connection is received
 * @param[in] p_usr_arg the p_usr_arg passed to silc_socket_create
 * @param[in] p_conn    the new connection
 * @param[in] p_conn_arg    user argument of this conneciton
 * @param[in] p_data_rng_buf    ring buffer that contains received data
 * @return 0 for success, 1 if want to break out the data_receive loop, -1 if want to close this connection
 *
 * the infrastructure will call this callback if new data has been received on a connection.
 * The data is contained in the p_data_rng_buf, the user need to pop the data out of the ring buffer to free
 * up space for further receiving. if the data is not complete, the user may choose to leave the non-complete
 * data inside the ring buffer and return 0, the data will be kept intact, and new data will be appended to existing
 * data. The callback will only be called again only if there's new data available.
 * Be aware, the size of the ring buffer is the size specified in the silc_socket_opt structure. The user should
 * always check if the ringbuffer is full, if the user can't process that data under such case, the only
 * reasonable choice would be return -1(close the connection).
 **/

int silc_test_socket_callback_conn_idle(silc_socket* p_socket, void* p_svr_arg, silc_sock_conn_id p_conn, void* p_conn_arg)
{
	const silc_socket_opt* opt = silc_socket_get_opt(p_socket);
	if(opt->socket_type == SILC_SOCKET_TYPE_TCP_CLIENT)
	{
		uint32_t size = 100;
		char* rechar = (char*)malloc(size);
		memset(rechar,'1',size - 1);
		*(rechar + size - 1) = '\0';
		if(0 != silc_sock_conn_send(p_conn,rechar,size))
		{
			printf("silc_test_socket_callback_conn_idle, SILC_SOCKET_TYPE_TCP_CLIENT send data failed\n");
			return 1;
		}
	}
    return 0;
}

int silc_test_socket_callback_data_read( silc_socket* p_socket, void* p_svr_arg, silc_sock_conn_id p_conn, void* p_conn_arg, silc_rng_buf_id p_data_rng_buf)
{
	char buf[SILC_TEST_SOCKET_BUF_LEN] = {0};
	uint32_t size;
	uint32_t bug_len = silc_rng_buf_get_length(p_data_rng_buf);
	const silc_socket_opt* opt = silc_socket_get_opt(p_socket);
	if(opt->socket_type == SILC_SOCKET_TYPE_TCP_SERVER || opt->socket_type == SILC_SOCKET_TYPE_UNIX_SERVER)
	{
		printf("silc_test_socket_callback_data_read, SILC_SOCKET_TYPE_TCP_SERVER receive data,bug_len = %u \n",bug_len);
		size = 550;
		if(bug_len >= size)
		{
			char* rechar = "receive bytes";
			if(0 != silc_sock_conn_send(p_conn,rechar, strlen(rechar) + 1))
			{
				printf("silc_test_socket_callback_data_read, SILC_SOCKET_TYPE_TCP_SERVER send data failed\n");
				return -1;
			}
		}
		while(bug_len >= size)
		{
			if(0 != silc_rng_buf_pop(p_data_rng_buf, buf, size))
			{
				printf("silc_test_socket_callback_data_read, silc_rng_buf_pop failed\n");
				return -1;
			}
			buf[size] = '\0';
			bug_len = silc_rng_buf_get_length(p_data_rng_buf);
			printf("silc_test_socket_callback_data_read, SILC_SOCKET_TYPE_TCP_SERVER receive data length = %d,bug_len = %u:\n"
					"%s\n",(int)strlen(buf),bug_len,buf);
		}
	}
	else if(opt->socket_type == SILC_SOCKET_TYPE_TCP_CLIENT ||opt->socket_type == SILC_SOCKET_TYPE_UNIX_CLIENT )
	{
		size = SILC_TEST_SOCKET_BUF_LEN - 1;
		while(bug_len > 0)
		{
			if(bug_len <= size)
				size = bug_len;
			if(0 != silc_rng_buf_pop(p_data_rng_buf, buf, size))
			{
				printf("silc_test_socket_callback_data_read, silc_rng_buf_pop failed\n");
				return -1;
			}
			buf[size] = '\0';
			printf("silc_test_socket_callback_data_read, SILC_SOCKET_TYPE_TCP_CLIENT receive data:\n"
								"%s\n",buf);
			bug_len -= size;
		}
	}
	else
	{
		printf("silc_test_socket_callback_data_read, error socket_type\n");
		return 1;
	}
	sleep(1);
	return 0;
}

int silc_test_socket_callback_conn_idle_break_loop(silc_socket* p_socket, void* p_svr_arg, silc_sock_conn_id p_conn, void* p_conn_arg)
{
	return 1;
}

void silc_test_socket_client_shutdown_loop(silc_socket_opt* opt,uint32_t create_recv_rng)
{
	printf("silc_test_socket_client_shutdown_loop\n");
	while(create_recv_rng--)
	{
		silc_socket* clt = silc_socket_create(opt, "my client");
		//printf("listen_sock = %d, max_fd = %d, p_usr_data = %s, socket_type = %d\n",clt->listen_sock,clt->max_fd,(char*)(clt->p_usr_data),clt->socket_type);
		int ret = silc_socket_tcp_client_loop(clt);
		printf("silc_test_socket_client,silc_socket_tcp_client_loop return code = %d\n",ret);
		silc_socket_client_close_curr_conn(clt);
	}
	return;
}

void silc_test_socket_server(silc_socket_opt* opt,uint32_t create_recv_rng)
{
	printf("silc_test_socket_server start\n");
	silc_socket* svr = silc_socket_create(opt, "my server");
	if(svr == NULL)
	{
		printf("silc_test_socket_server, svr is NULL\n");
		return;
	}
   // printf("listen_sock = %d, max_fd = %d, p_usr_data = %s, socket_type = %d\n",svr->listen_sock,svr->max_fd,(char*)(svr->p_usr_data),svr->socket_type);
    //silc_sock_conn_t* svr_conn = silc_socket_tcp_accept_connection(svr);
  //  printf("silc_test_socket_server,svr_conn->sock = %d\n",svr_conn->sock);
    int ret = silc_socket_data_recv_loop(svr);
    printf("silc_test_socket_server, silc_socket_data_recv_loop ret = %d\n",ret);
    silc_socket_client_close_curr_conn(svr);
}

void silc_test_socket_client(silc_socket_opt* opt,uint32_t create_recv_rng)
{
	silc_socket* clt = NULL;
	clt = silc_socket_create(opt, "my client");
	printf("silc_test_socket_client start \n");
	//printf("listen_sock = %d, max_fd = %d, p_usr_data = %s, socket_type = %d\n",clt->listen_sock,clt->max_fd,(char*)(clt->p_usr_data),clt->socket_type);
	int ret = silc_socket_tcp_client_loop(clt);
	printf("silc_test_socket_client,silc_socket_tcp_client_loop return code = %d\n",ret);
	silc_socket_client_close_curr_conn(clt);
}

void silc_test_socket_unix_server(silc_socket_opt* opt,uint32_t create_recv_rng)
{
	printf("silc_test_socket_unix_server start\n");
	printf("sock type is %d \n",opt->socket_type);
	silc_socket* svr = silc_socket_create(opt, "my unix server");
	if(svr == NULL)
	{
		printf("silc_test_socket_server, svr is NULL\n");
		return;
	}
   // printf("listen_sock = %d, max_fd = %d, p_usr_data = %s, socket_type = %d\n",svr->listen_sock,svr->max_fd,(char*)(svr->p_usr_data),svr->socket_type);
    //silc_sock_conn_t* svr_conn = silc_socket_tcp_accept_connection(svr);
  //  printf("silc_test_socket_server,svr_conn->sock = %d\n",svr_conn->sock);
	// ret = silc_socket_unix_data_recv_loop(svr);
    int ret = silc_socket_data_recv_loop(svr);
    printf("silc_test_socket_unix_server, silc_socket_data_recv_loop ret = %d\n",ret);
    silc_socket_client_close_curr_conn(svr);
}

void silc_test_socket_unix_client(silc_socket_opt* opt,uint32_t create_recv_rng)
{
	silc_socket* clt = NULL;
	clt = silc_socket_create(opt, "my unix client");
	//printf("listen_sock = %d, max_fd = %d, p_usr_data = %s, socket_type = %d\n",clt->listen_sock,clt->max_fd,(char*)(clt->p_usr_data),clt->socket_type);
	int ret = silc_socket_tcp_client_loop(clt);
	printf("silc_test_socket_unix_client,silc_socket_tcp_client_loop return code = %d\n",ret);
	silc_socket_client_close_curr_conn(clt);
}

void silc_test_socket_access(test_lib_arg* arg,int* code_array,int array_length)
{
	uint32_t i = 0;
	silc_bool is_tcp_svr = silc_false;
	silc_bool is_tcp_client = silc_false;
	silc_bool is_unix_svr = silc_false;
	silc_bool is_unix_client = silc_false;
	silc_socket_opt s_opt;

	process_socket socket_fuc = NULL;

	s_opt.callback_conn_close = silc_test_socket_callback_conn_close;
	s_opt.callback_conn_new = silc_test_socket_callback_conn_new;
	s_opt.callback_data_read = silc_test_socket_callback_data_read;
	s_opt.callback_conn_idle = silc_test_socket_callback_conn_idle;
	while(code_array[i] >= 0)
	{
		switch (code_array[i])
		{
	        case TEST_SOCKET_SERVER:
	        	if(is_tcp_client)
	        	{
	        		printf("not both test server and client\n");
	        		return;
	        	}
	        	printf("test server\n");
	        	is_tcp_svr = silc_true;
	        	s_opt.socket_type = SILC_SOCKET_TYPE_TCP_SERVER;
	        	socket_fuc = silc_test_socket_server;
	        	break;
	        case TEST_SOCKET_CLIENT:
	        	if(is_tcp_svr)
	        	{
	        		printf("not both test server and client\n");
	        		return;
	        	}
	        	printf("test client\n");
	        	socket_fuc = silc_test_socket_client;
	        	s_opt.socket_type = SILC_SOCKET_TYPE_TCP_CLIENT;
	        	is_tcp_client = silc_true;
	        	break;
		    case TEST_SOCKET_IP:
		    	if(is_tcp_client)
		    	{
		    		strcpy(s_opt.o.tcp_client.remote_host,arg->socket_ip);
		    		s_opt.o.tcp_client.remote_host[strlen(arg->socket_ip) + 1] = '\0';
		    		printf("TEST_SOCKET_IP remote_host = %s\n",s_opt.o.tcp_client.remote_host);
		    	}
		    	else
		    	{
		    		printf("if set socket ip,test client only\n");
		    		return;
		    	}
		    	break;
		    case TEST_SOCKET_UNIX_SERVER:
	        	if(is_unix_client)
	        	{
	        		printf("not both test server and client\n");
	        		return;
	        	}
	        	printf("test server\n");
	        	is_unix_svr = silc_true;
	        	s_opt.socket_type = SILC_SOCKET_TYPE_UNIX_SERVER;
	        	socket_fuc = silc_test_socket_unix_server;
	        	break;
		    case TEST_SOCKET_UNIX_CLIENT:
	        	if(is_unix_svr)
	        	{
	        		printf("not both test server and client\n");
	        		return;
	        	}
	        	printf("test server\n");
	        	s_opt.socket_type = SILC_SOCKET_TYPE_UNIX_CLIENT;
	        	socket_fuc = silc_test_socket_unix_client;
	        	is_unix_client = silc_true;
	        	break;
		    case TEST_SOCKET_UNIX_DOMAIN:
		    	if(is_unix_client)
		    	{
		    		strcpy(s_opt.o.unix_client.unix_domain,arg->socket_unix_domain);
		    		s_opt.o.unix_client.unix_domain[strlen(arg->socket_unix_domain) + 1] = '\0';
		    		printf("TEST_SOCKET_DOMAIN unix client domain = %s\n",s_opt.o.unix_client.unix_domain);
		    	}
		    	else if(is_unix_svr)
		    	{
		    		strcpy(s_opt.o.unix_svr.unix_domain,arg->socket_unix_domain);
		    		s_opt.o.unix_svr.unix_domain[strlen(arg->socket_unix_domain) + 1] = '\0';
		    		printf("TEST_SOCKET_DOMAIN unix svr domain = %s\n",s_opt.o.unix_svr.unix_domain);
		    	}
		    	break;
		    case TEST_SOCKET_PORT:
		    	s_opt.port_num = arg->socket_port;
		    	printf("TEST_SOCKET_PORT port_num = %u\n",s_opt.port_num);
		    	break;
		    case TEST_SOCKET_BUF_SIZE:
		    	s_opt.max_buf_size = arg->socket_buf_szie;
		    	printf("TEST_SOCKET_BUF_SIZE max_buf_size = %u\n",s_opt.max_buf_size);
		   		break;
		    case TEST_SOCKET_SVR_DUP_IP:
		    	SILC_TEST_SOCKET_OPT_O(TEST_SOCKET_SVR_DUP_IP,tcp_svr,allow_duplicate_ip,arg->allow_duplicate_ip)
				break;
		    case TEST_SOCKET_SVR_SGLTHRD:
		    	//SILC_TEST_SOCKET_OPT_O(TEST_SOCKET_SVR_SGLTHRD,tcp_svr,single_thread,arg->svr_sgl_thrd)
				SILC_TEST_SOCKET_OPT_O(TEST_SOCKET_SVR_SGLTHRD,unix_svr,single_thread,arg->svr_sgl_thrd)
				break;
		    case TEST_SOCKET_CLNT_RECONNECT:
		    	SILC_TEST_SOCKET_OPT_O(TEST_SOCKET_CLNT_RECONNECT,tcp_client,reconnect,arg->clnt_reconnect)
				break;
		    case TEST_SOCKET_CLNT_QUDISCONN:
		    	SILC_TEST_SOCKET_OPT_O(TEST_SOCKET_CLNT_QUDISCONN,tcp_client,quit_on_disconnect,arg->clnt_qu_on_disconn)
				break;
		    case TEST_SOCKET_CLNT_RBUFF:
		    	if(is_tcp_client)
		    	{
		    		if(arg->clnt_use_rbuf != 0)
		    		{
		    			s_opt.callback_conn_idle = silc_test_socket_callback_conn_idle_break_loop;
		    			socket_fuc = silc_test_socket_client_shutdown_loop;
		    		}

		    		printf("TEST_SOCKET_CLNT_RBUFF tcp_client,arg->clnt_use_rbuf = %d\n",arg->clnt_use_rbuf);
		    	}
		    	else
		    	{
		    		printf("if set create_recv_rng,test tcp_client only\n");
		    		return;
		    	}
				break;
		    case TEST_SOCKET_CLNT_SEND_TIME:
		    	s_opt.idle_interval.tv_sec = arg->clnt_time_send;
		    	s_opt.idle_interval.tv_usec = 0;
		    	printf("TEST_SOCKET_CLNT_SEND_TIME tcp_client, idle_interval = %ld\n",s_opt.idle_interval.tv_sec);
		        break;
		    default:
		    	printf("err argv\n");
		    	return;
		}
		i++;
	}

	if(socket_fuc != NULL)
	{
		socket_fuc(&s_opt,arg->clnt_use_rbuf);
	}
	else
	{
		printf("socket_fuc is NULL\n");
	}
}
