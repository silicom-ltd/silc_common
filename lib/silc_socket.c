/*
 * silc_socket.c
 *
 *  Created on: Nov 22, 2010
 *      Author: jeff_zheng
 *   Copyright: NetPerform Technology
 */

#include <inttypes.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <netdb.h>
#include <sys/un.h>
#include "silc_common.h"
#include "silc_common/silc_socket.h"

struct silc_socket_s
{
	silc_socket_type			socket_type;
	silc_socket_opt	 		opt;

	void*					p_usr_data;
	int						listen_sock;
	int						wake_pipes[2];
	int						max_fd;
	fd_set					fdset_backup;
	fd_set					fdset_sel;
	silc_list					conn_list;
	silc_bool					running;
	silc_sem					sleep_sem;
};


struct silc_sock_conn_s
{
	silc_list_node			list_node;
	struct sockaddr_in 		_remote_saddr;
	struct sockaddr_in6 	_remote_saddr6;
	struct sockaddr_un      unix_saddr;
	struct sockaddr* 		p_remote_saddr;

	int 					saddr_len;

	silc_bool					connected;
	uint32_t				remote_ip;
	uint16_t				remote_port;
	int						sock;
	silc_rng_buf_id			rcv_rng;
	void*					p_usr_data;
};


silc_sock_conn* silc_sock_conn_create(uint32_t max_buf_size, silc_bool create_recv_rng);
int silc_sock_conn_close(silc_socket* p_tcp, silc_sock_conn* p_conn);



#define SILC_SOCKET_BUF_SIZE_DEFAULT    10240

static int silc_sock_conn_data_recv(silc_sock_conn* p_conn);

#define SILC_SOCK_DBG
#ifdef SILC_SOCK_DBG
#define SILC_SOCK_DBG_LOG	SILC_LOG
#define SILC_SOCK_DBG_ERR	SILC_ERR
#else
#define SILC_SOCK_DBG_LOG(...)	do {} while(0)
#define SILC_SOCK_DBG_ERR(...) do {} while(0)
#endif

#define silc_sock_opt_check_msg(condition, fmt, ...) \
	if(condition)	\
	{	\
		SILC_SOCK_DBG_ERR(fmt,## __VA_ARGS__); \
		return -1; \
	}

int silc_socket_opt_check(silc_socket_opt* opts)
{
	switch(opts->socket_type)
	{
	case SILC_SOCKET_TYPE_TCP_SERVER:
		silc_sock_opt_check_msg(opts->port_num==0, "tcp server: Invalid port number:%u",opts->port_num);
		silc_sock_opt_check_msg(opts->o.tcp_svr.single_thread && opts->callback_data_read==NULL,
								"tcp server: single thread model needs a data callback");
		silc_sock_opt_check_msg(opts->o.tcp_svr.single_thread && opts->callback_data_read==NULL,
								"tcp server: single thread model needs a data callback");
		silc_sock_opt_check_msg((!opts->o.tcp_svr.single_thread) && opts->callback_data_read!=NULL,
								"tcp server: multi thread model doesn't needs a data callback");
		break;
	case SILC_SOCKET_TYPE_TCP_CLIENT:
		silc_sock_opt_check_msg(opts->port_num==0, "tcp client: Invalid port number:%u",opts->port_num);
		silc_sock_opt_check_msg(opts->callback_data_read==NULL, "tcp client: needs a data callback");
		break;
	case SILC_SOCKET_TYPE_UNIX_SERVER:
		silc_sock_opt_check_msg(opts->o.unix_svr.single_thread && opts->callback_data_read==NULL,
								"unix server: single thread model needs a data callback");
		silc_sock_opt_check_msg(opts->o.unix_svr.single_thread && opts->callback_data_read==NULL,
								"unix server: single thread model needs a data callback");
		silc_sock_opt_check_msg((!opts->o.unix_svr.single_thread) && opts->callback_data_read!=NULL,
								"unix server: multi thread model doesn't needs a data callback");
		break;
	case SILC_SOCKET_TYPE_UNIX_CLIENT:
		silc_sock_opt_check_msg(opts->o.unix_client.unix_domain==NULL, "unix client: Invalid unix domain:%s",opts->o.unix_client.unix_domain);
		silc_sock_opt_check_msg(opts->callback_data_read==NULL, "unix client: needs a data callback");
		break;
	}

	return 0;
}

int silc_socket_init_tcp_client(silc_socket* p_socket)
{
	return 0;
}

int silc_socket_init_unix_client(silc_socket* p_socket)
{

	return 0;
}

static int silc_socket_bind_tcp_svr(silc_socket* p_svr)
{
	struct sockaddr_in local_saddr;
	socklen_t local_saddr_len;
	int ret;

	memset(&local_saddr, 0, sizeof(local_saddr));
	if(strlen(p_svr->opt.o.tcp_svr.bind_address) == 0)
		local_saddr.sin_addr.s_addr = INADDR_ANY;
	else
		local_saddr.sin_addr.s_addr = inet_addr(p_svr->opt.o.tcp_svr.bind_address);
	local_saddr.sin_port = htons(p_svr->opt.port_num);
	local_saddr.sin_family = AF_INET;
	local_saddr_len = sizeof(struct sockaddr_in);

	ret = bind(p_svr->listen_sock, (struct sockaddr *)(&local_saddr), local_saddr_len);
	if(ret != 0)
	{
		SILC_SOCK_DBG_ERR("bind failed, sock: %d, port_num:%d: %s\n",p_svr->listen_sock, p_svr->opt.port_num, strerror(errno));
		return -1;
	}

	ret = listen(p_svr->listen_sock,1);
	if(ret!=0)
	{
		SILC_SOCK_DBG_ERR("listen failed, sock: %d, port_num:%d: %s\n",p_svr->listen_sock, p_svr->opt.port_num, strerror(errno));
		return -1;
	}

	return 0;
}

static int silc_socket_bind_unix_svr(silc_socket* p_svr)
{
	struct sockaddr_un unix_saddr;
	socklen_t unix_saddr_len;
	int ret;

	memset(&unix_saddr, 0, sizeof(struct sockaddr_un));
	unix_saddr.sun_family = AF_UNIX;
	strcpy(unix_saddr.sun_path,p_svr->opt.o.unix_svr.unix_domain);
	unix_saddr_len = sizeof(struct sockaddr_un);

	ret = bind(p_svr->listen_sock, (struct sockaddr_un *)(&unix_saddr), unix_saddr_len);
	if(ret != 0)
	{
		SILC_SOCK_DBG_ERR("bind failed, unix sock: %d, unix domain:%s: %s\n",p_svr->listen_sock,p_svr->opt.o.unix_svr.unix_domain, strerror(errno));
		return -1;
	}
	if(0 != chmod(unix_saddr.sun_path,S_IRWXO|S_IRWXG|S_IRWXU))
	{
		SILC_SOCK_DBG_ERR("chmod unix socket %s fail error : %s\n",unix_saddr.sun_path,strerror(errno));
		return -1;
	}
	ret = listen(p_svr->listen_sock,1);
	if(ret!=0)
	{
		SILC_SOCK_DBG_ERR("listen failed, unix sock: %d, unix domain:%s: %s\n",p_svr->listen_sock,p_svr->opt.o.unix_svr.unix_domain, strerror(errno));
		return -1;
	}

	return 0;
}

int silc_socket_init_tcp_server(silc_socket* p_socket)
{
	int ret, setoptarg;

	p_socket->listen_sock = socket(AF_INET,SOCK_STREAM, 0);
	if(p_socket->listen_sock==-1)
	{
		SILC_SOCK_DBG_ERR("create listening socket failed: %s\n",strerror(errno));
		return -1;
	}
	setoptarg=1;
	/*we'll reuse addresses, in case the machine reboot and reconnect quickly*/
	ret = setsockopt(p_socket->listen_sock,SOL_SOCKET,SO_REUSEADDR,&setoptarg,sizeof(setoptarg));
	if(ret!=0)
	{
		SILC_SOCK_DBG_LOG("setsockopt return %d %s",ret,strerror(errno));
	}

	if(0!=silc_socket_bind_tcp_svr(p_socket))
	{
		SILC_SOCK_DBG_ERR("Can not bind to port %u", p_socket->opt.port_num);
		close(p_socket->listen_sock);
		return -1;
	}

	//configure listen sock with select
	FD_SET(p_socket->listen_sock, &(p_socket->fdset_backup));

	p_socket->max_fd = p_socket->listen_sock;
	return 0;
}

int silc_socket_init_unix_server(silc_socket* p_socket)
{
	int ret, setoptarg;
	//int socket_remained;
    /*
     * In case the program exited inadvertently on the last run,
     * remove the socket.
     */
	if ( !access(p_socket->opt.o.unix_svr.unix_domain,F_OK) )
	{
		ret = unlink(p_socket->opt.o.unix_svr.unix_domain);
		if(ret < 0)
		{
			SILC_SOCK_DBG_ERR("unlink to %s err:%s", p_socket->opt.o.unix_svr.unix_domain,strerror(errno));
		}
	}
	p_socket->listen_sock = socket(PF_UNIX,SOCK_STREAM, 0);
	if(p_socket->listen_sock==-1)
	{
		SILC_SOCK_DBG_ERR("create listening socket failed: %s\n",strerror(errno));
		return -1;
	}
	setoptarg=1;
	/*we'll reuse addresses, in case the machine reboot and reconnect quickly*/
	ret = setsockopt(p_socket->listen_sock,SOL_SOCKET,SO_REUSEADDR,&setoptarg,sizeof(setoptarg));
	if(ret!=0)
	{
		SILC_SOCK_DBG_LOG("setsockopt return %d %s",ret,strerror(errno));
	}
	/*we will delete the socket domain if there are the same one */
	if(0!=silc_socket_bind_unix_svr(p_socket))
	{
		SILC_SOCK_DBG_ERR("Can not bind to %s", p_socket->opt.o.unix_svr.unix_domain);
		close(p_socket->listen_sock);
		unlink(p_socket->opt.o.unix_svr.unix_domain);
		return -1;
	}

	//configure listen sock with select
	FD_SET(p_socket->listen_sock, &(p_socket->fdset_backup));

	p_socket->max_fd = p_socket->listen_sock;
	return 0;
}

const silc_socket_opt* silc_socket_get_opt(silc_socket* p_sock)
{
	return &p_sock->opt;
}


silc_socket* silc_socket_create(silc_socket_opt* opts, void* p_usr_data)
{
	int ret;
	silc_socket* p_socket;

	if(silc_socket_opt_check(opts)!=0)
		return NULL;

	p_socket = malloc(sizeof(*p_socket));
	if(p_socket==NULL)
		return NULL;
	memset(p_socket, 0, sizeof(*p_socket));
	p_socket->listen_sock = -1;
	p_socket->max_fd = -1;
	p_socket->wake_pipes[0] = -1;
	p_socket->wake_pipes[1] = -1;
	if(0!=pipe(p_socket->wake_pipes))
	{
		free(p_socket);
		return NULL;
	}

	p_socket->p_usr_data = p_usr_data;
	p_socket->opt = *opts;
	silc_sem_init(&p_socket->sleep_sem);

	/*this buffer size is used by each sock_connection*/
	if(opts->max_buf_size)
		p_socket->opt.max_buf_size = opts->max_buf_size;
	else
		p_socket->opt.max_buf_size = SILC_SOCKET_BUF_SIZE_DEFAULT;

	if(p_socket->opt.idle_interval.tv_sec==0&&p_socket->opt.idle_interval.tv_usec==0)
	{//default 100ms idel interval
		p_socket->opt.idle_interval.tv_sec = 0;
		p_socket->opt.idle_interval.tv_usec = 100000;
	}

	/*initialize main connection list*/
	silc_list_init(&(p_socket->conn_list));

	/*initialize select related members*/
	FD_ZERO(&(p_socket->fdset_backup));
	FD_ZERO(&(p_socket->fdset_sel));

	//set our wake pipe into backup fdset
	FD_SET(p_socket->wake_pipes[0], &(p_socket->fdset_backup));
	switch(p_socket->opt.socket_type)
	{
	case SILC_SOCKET_TYPE_TCP_SERVER:
		ret=silc_socket_init_tcp_server(p_socket);
		break;
	case SILC_SOCKET_TYPE_TCP_CLIENT:
		ret=silc_socket_init_tcp_client(p_socket);
		break;
	case SILC_SOCKET_TYPE_UNIX_SERVER:
		ret=silc_socket_init_unix_server(p_socket);
		break;
	case SILC_SOCKET_TYPE_UNIX_CLIENT:
		ret=silc_socket_init_unix_client(p_socket);
		break;
	default:
		ret = -1;
		break;
	}
	if(ret!=0)
	{
		close(p_socket->wake_pipes[0]);
		close(p_socket->wake_pipes[1]);
		free(p_socket);
		return NULL;
	}
	p_socket->running = silc_true;
	return p_socket;
}

void silc_socket_stop(silc_socket* p_socket)
{
	p_socket->running = silc_false;
	//wake up any thread
	write(p_socket->wake_pipes[1], p_socket, 1);
	silc_sem_give(&p_socket->sleep_sem);
}

void silc_socket_destroy(silc_socket* p_socket)
{
	silc_sock_conn* p_conn, * p_tmp;
	if(p_socket->running)
	{
		SILC_ERR("Destroying socket %p without stopping it", p_socket);
		silc_socket_stop(p_socket);
		//force wait other thread
		silc_time_sleep(0, 1000);
	}

	silc_list_for_each_entry_safe(p_conn, p_tmp, &p_socket->conn_list, list_node)
	{
		silc_list_del(&p_conn->list_node);
		silc_sock_conn_close(p_socket, p_conn);
	}

	if(p_socket->listen_sock != -1)
	{
		close(p_socket->listen_sock);
		close(p_socket->wake_pipes[0]);
		close(p_socket->wake_pipes[1]);
	}
	return;
}

int silc_socket_check_max_fd(silc_socket* p_socket)
{
	int max_fd;
	silc_sock_conn* p_conn;
	max_fd = p_socket->listen_sock;
	if(max_fd < p_socket->wake_pipes[0])
		max_fd = p_socket->wake_pipes[0];

	if(p_socket->opt.socket_type==SILC_SOCKET_TYPE_TCP_CLIENT|| p_socket->opt.o.tcp_svr.single_thread)
	{
		//only calculate all fds in single thread mode
		silc_list_for_each_entry(p_conn, &(p_socket->conn_list), list_node)
		{
			if(max_fd<p_conn->sock)
			{
				max_fd = p_conn->sock;
			}
		}
	}
	p_socket->max_fd = max_fd;
	return 0;
}

int silc_socket_tcp_accept_connection(silc_socket* p_svr)
{
	int ret;
	struct sockaddr_in peer_saddr;
	socklen_t peer_saddr_len;
	silc_sock_conn* p_conn;
	int usr_ret;

#ifdef SILC_SOCK_DBG
	silc_str_declare(iptmp, 24);
#endif

	p_conn = silc_sock_conn_create(p_svr->opt.max_buf_size, p_svr->opt.o.tcp_svr.single_thread);
	if(p_conn==NULL)
	{
		return -1;
	}

	peer_saddr_len = sizeof(peer_saddr);
	ret = accept(p_svr->listen_sock, (struct sockaddr *)&peer_saddr, &peer_saddr_len);
	if(ret==-1)
	{
		SILC_SOCK_DBG_ERR("accept error: %s",strerror(errno));
		silc_sock_conn_close(p_svr, p_conn);
		return -1;
	}

	p_conn->remote_ip = peer_saddr.sin_addr.s_addr;
	p_conn->remote_port = peer_saddr.sin_port;
	p_conn->sock = ret;
	SILC_SOCK_DBG_LOG("new tcp connection from %s:%d", silc_str_from_ip(&iptmp, 24, p_conn->remote_ip), ntohs(p_conn->remote_port));

	/*get user to know the connection*/
	usr_ret =p_svr->opt.callback_conn_new(p_svr, p_svr->p_usr_data, p_conn);
	if(usr_ret==-1)
	{/*user has either an error, or don't want this connection, close it*/
		SILC_SOCK_DBG_ERR("close this connection \n");
		silc_sock_conn_close(p_svr, p_conn);
		//this is not a lib error, so we set usr_ret to 0 for returning
		usr_ret = 0;
	}

	silc_list_add_tail(&(p_conn->list_node),&(p_svr->conn_list));
	if(p_svr->opt.o.tcp_svr.single_thread)
	{
		if(p_svr->max_fd < p_conn->sock)
			p_svr->max_fd = p_conn->sock;
		FD_SET(p_conn->sock, &(p_svr->fdset_backup));
	}
	p_conn->connected = silc_true;
    return usr_ret;
}

int silc_socket_unix_accept_connection(silc_socket* p_svr)
{
	int ret;
	struct sockaddr_un unix_saddr;
	socklen_t unix_saddr_len;
	silc_sock_conn* p_conn;
	int usr_ret;

#ifdef SILC_SOCK_DBG
	//silc_str_declare(iptmp, 24);
#endif

	p_conn = silc_sock_conn_create(p_svr->opt.max_buf_size, p_svr->opt.o.unix_svr.single_thread);
	if(p_conn==NULL)
	{
		return -1;
	}

	unix_saddr_len = sizeof(unix_saddr);
	ret = accept(p_svr->listen_sock, (struct sockaddr *)&unix_saddr, &unix_saddr_len);
	if(ret==-1)
	{
		SILC_SOCK_DBG_ERR("accept error: %s",strerror(errno));
		silc_sock_conn_close(p_svr, p_conn);
		return -1;
	}

	//p_conn->remote_ip = peer_saddr.sin_addr.s_addr;
	//p_conn->remote_port = peer_saddr.sin_port;
	p_conn->unix_saddr.sun_family = AF_UNIX;
	strcpy(p_conn->unix_saddr.sun_path,p_svr->opt.o.unix_svr.unix_domain);
	p_conn->sock = ret;
	/*get user to know the connection*/
	usr_ret =p_svr->opt.callback_conn_new(p_svr, p_svr->p_usr_data, p_conn);
	if(usr_ret==-1)
	{/*user has either an error, or don't want this connection, close it*/
		SILC_SOCK_DBG_ERR("close this connection \n");
		silc_sock_conn_close(p_svr, p_conn);
		//this is not a lib error, so we set usr_ret to 0 for returning
		usr_ret = 0;
	}

	silc_list_add_tail(&(p_conn->list_node),&(p_svr->conn_list));
	if(p_svr->opt.o.unix_svr.single_thread)
	{
		if(p_svr->max_fd < p_conn->sock)
			p_svr->max_fd = p_conn->sock;
		FD_SET(p_conn->sock, &(p_svr->fdset_backup));
	}
	p_conn->connected = silc_true;
    return usr_ret;
}

int silc_socket_data_receive(silc_socket* p_tcp)
{
	silc_sock_conn* p_conn, *p_conn_safe;

	SILC_INFO("[SILC_SOCK]Try receive data");
	int usr_proc_ret;
	silc_list_for_each_entry_safe(p_conn, p_conn_safe, &(p_tcp->conn_list), list_node)
	{
		if(FD_ISSET(p_conn->sock, &(p_tcp->fdset_sel)))
		{
			/*implement fair scheduling, this will cause deadloop
			 * TODO: Shall put the conn onto a separate list and recat the list to the conn_list.
			 **/
//			silc_list_del(&p_conn->list_node);
//			silc_list_add_tail(&p_conn->list_node, &(p_tcp->conn_list));

			if(0!=silc_sock_conn_data_recv(p_conn))
			{/*peer has closed the connection, tell caller to shutdown the connection*/
				/*either something has gone wrong, or peer has closed the connection*/
				/*we'll remove this connection from the active connection list*/
				//SILC_SOCK_DBG_LOG("0!=silc_sock_conn_data_recv()  close socket");
				FD_CLR(p_conn->sock, &(p_tcp->fdset_backup));
				silc_list_del(&(p_conn->list_node));
				silc_sock_conn_close(p_tcp, p_conn);
				silc_socket_check_max_fd(p_tcp);
				return -1;
			}
			usr_proc_ret = p_tcp->opt.callback_data_read(p_tcp, p_tcp->p_usr_data, p_conn, p_conn->p_usr_data, p_conn->rcv_rng);
			if(usr_proc_ret<0)
			{/*error in user data processing, tell caller to close connection*/
				SILC_SOCK_DBG_ERR("[SOCK][%"PRIxPTR"] Error when processing received data from connection ", (uintptr_t)p_conn);
				FD_CLR(p_conn->sock, &(p_tcp->fdset_backup));
				silc_list_del(&(p_conn->list_node));
				silc_sock_conn_close(p_tcp, p_conn);
				silc_socket_check_max_fd(p_tcp);
				//TODO: this sleep is arbitrary, we should make it configurable
				//second before making another connection attempt
				silc_sem_wait(&p_tcp->sleep_sem, 100000);
				return -1;
			}
			if(usr_proc_ret>0)
			{//user want to interrupt receiving
				return 1;
			}
		}
	}
	return 0;
}

int silc_socket_idle_process(silc_socket* p_tcp)
{
	silc_sock_conn* p_conn, *p_conn_safe;
	if(p_tcp->opt.callback_conn_idle==NULL)
		return 0;
	silc_bool breakloop = silc_false;
	silc_list_for_each_entry_safe(p_conn, p_conn_safe, &(p_tcp->conn_list), list_node)
	{
		int ret = p_tcp->opt.callback_conn_idle(p_tcp, p_tcp->p_usr_data, p_conn, p_conn->p_usr_data);
		if(ret==1)
		{
			breakloop = silc_true;
		}
		else if(ret==-1)
		{//user want to shutdown the connection
			silc_list_del(&(p_conn->list_node));
			silc_sock_conn_close(p_tcp, p_conn);
			silc_socket_check_max_fd(p_tcp);
			SILC_SOCK_DBG_ERR("silc_socket_idle_process close socket \n");
		}
	}
	if(breakloop)
	{
		return 1;
	}
	return 0;
}

int silc_socket_data_recv_loop(silc_socket* p_tcp)
{
	struct timeval sel_tmo;

	int ret;
	int usr_ret = 0;

	while(p_tcp->max_fd>0 && p_tcp->running)
	{
		//we need to restore the whole fdset everytime, from a backup copy
		memcpy(&p_tcp->fdset_sel,&p_tcp->fdset_backup,sizeof(p_tcp->fdset_backup));
		sel_tmo.tv_sec = p_tcp->opt.idle_interval.tv_sec;
		sel_tmo.tv_usec = p_tcp->opt.idle_interval.tv_usec;
		ret = select(p_tcp->max_fd + 1, &(p_tcp->fdset_sel), NULL, NULL, &sel_tmo);
		if(ret==0)
		{/*timeout*/
			if(silc_socket_idle_process(p_tcp)==1)
				break;
			continue;
		}
		if(ret<0)
		{/*error*/
			SILC_SOCK_DBG_ERR("select error: %s\n",strerror(errno));
			//TODO: this sleep is arbitrary, we should make it configurable
			//second before making another connection attempt
			silc_sem_wait(&p_tcp->sleep_sem, 100000);
			continue;
		}
		if(p_tcp->listen_sock>0)
		{//only valid for a server
			if(FD_ISSET(p_tcp->listen_sock,  &(p_tcp->fdset_sel)))
			{/*new connection*/
				FD_CLR(p_tcp->listen_sock,  &(p_tcp->fdset_sel));
				if(p_tcp->opt.socket_type == SILC_SOCKET_TYPE_TCP_SERVER)
				{
					usr_ret = silc_socket_tcp_accept_connection(p_tcp);
				}
				else if(p_tcp->opt.socket_type == SILC_SOCKET_TYPE_UNIX_SERVER)
				{
					usr_ret = silc_socket_unix_accept_connection(p_tcp);
				}
				else
				{
					SILC_SOCK_DBG_ERR("error: socket type : %d\n",p_tcp->opt.socket_type);
				}
				if(usr_ret>0)
				{//user want to interrupt receiving
					return 1;
				}
			}
		}
		if(p_tcp->opt.socket_type==SILC_SOCKET_TYPE_TCP_CLIENT||p_tcp->opt.o.tcp_svr.single_thread||p_tcp->opt.socket_type==SILC_SOCKET_TYPE_UNIX_CLIENT||p_tcp->opt.o.unix_svr.single_thread)
		{
			usr_ret=silc_socket_data_receive(p_tcp);
			if(usr_ret>0)
			{//user want to interrupt receiving
				return 1;
			}
		}
	}
	return 0;
}

silc_sock_conn* silc_sock_conn_create(uint32_t max_buf_size, silc_bool create_recv_rng)
{
	silc_sock_conn* p_conn;
	p_conn = malloc(sizeof(*p_conn));
	if(p_conn==NULL)
		return NULL;
	memset(p_conn, 0, sizeof(*p_conn));
//	SILC_TRACE("[SOCK][%"PRIx64"] Creating socket connection ", (uintptr_t)p_conn);

	p_conn->sock = -1;
	p_conn->connected = silc_false;

	if(create_recv_rng)
	{
		p_conn->rcv_rng= silc_rng_buf_create(1, max_buf_size, NULL);
		if(p_conn->rcv_rng==NULL)
		{
			free(p_conn);
			return NULL;
		}
	}
	return p_conn;
}

int silc_sock_conn_close(silc_socket* p_tcp, silc_sock_conn* p_conn)
{
	int usr_ret;
//	silc_str_declare(iptmp, 24);
	silc_socket_check_max_fd(p_tcp);

//	SILC_TRACE("[SOCK][%"PRIx64"] Closing socket connection ", (uintptr_t)p_conn);
	usr_ret = p_tcp->opt.callback_conn_close(p_tcp, p_tcp->p_usr_data, p_conn, p_conn->p_usr_data);
	if(p_conn->sock>=0)
	{
		shutdown(p_conn->sock, SHUT_RDWR);
		close(p_conn->sock);
//		SILC_TRACE("[SOCK][%"PRIx64"] Close TCP connection from %s:%d", (uintptr_t)p_conn,
//						silc_str_from_ip(&iptmp, 24, p_conn->remote_ip), ntohs(p_conn->remote_port));
	}
	if(p_tcp->opt.socket_type==SILC_SOCKET_TYPE_TCP_SERVER||p_tcp->opt.o.tcp_svr.single_thread)
	{
		silc_rng_buf_destroy(p_conn->rcv_rng);
		p_conn->rcv_rng = 0;
	}
	if(p_tcp->opt.socket_type==SILC_SOCKET_TYPE_UNIX_SERVER||p_tcp->opt.o.tcp_svr.single_thread)
	{
		silc_rng_buf_destroy(p_conn->rcv_rng);
		p_conn->rcv_rng = 0;
	}

	free(p_conn);
	return usr_ret;
}

void silc_sock_conn_set_arg(silc_sock_conn* p_conn, void* p_usr_arg)
{
	p_conn->p_usr_data = p_usr_arg;
	return;
}

int silc_sock_conn_get_sock(silc_sock_conn* p_conn)
{
	return p_conn->sock;
}

int silc_sock_conn_send(silc_sock_conn* p_conn, void* buffer, uint32_t size)
{
	int ret;
	ret = send(p_conn->sock, buffer, size, MSG_NOSIGNAL);
	if(ret==(int)size)
		return 0;
	if(ret<0)
	{
		SILC_SOCK_DBG_ERR("[SOCK][%"PRIxPTR"] vd_sock_write err: %s", (uintptr_t)p_conn, strerror(errno));
		return -1;
	}
	return 0;
}

/*return -1, if there is error and socket need to be closed,
  return 0 otherwise*/
int silc_sock_conn_data_recv(silc_sock_conn* p_conn)
{
	int ret;
	uint32_t size;
	uint8_t buf[2048];

	do
	{
		size = silc_rng_buf_get_free_count(p_conn->rcv_rng);
		if(size==0)
		{/*not enough buffer space for holding received data*/
			return 0;
		}
		if(size > sizeof(buf))
			size = sizeof(buf);
		ret = recv(p_conn->sock, buf, size, MSG_DONTWAIT);
		if(ret<0)
		{
			int err;
			err = errno;
			if(errno==ENOTCONN || errno == ENOTSOCK || errno == EBADF)
			{
				SILC_SOCK_DBG_ERR("[SOCK][%"PRIxPTR"] Error on receive %s", (uintptr_t)p_conn, strerror(errno));
				return -1;
			}
			if(err==EAGAIN)
			{
				SILC_INFO("[SOCK][%"PRIxPTR"] EAGAIN on receive", (uintptr_t)p_conn);
				return 0;
			}
		}
		else if(ret==0)
		{/*peer has shutdown*/
			SILC_INFO("[SOCK][%"PRIxPTR"] Peer has closed connection", (uintptr_t)p_conn);
			return -1;
		}
		else
		{
//        	SILC_SOCK_DBG_LOG("[SOCK][%"PRIx64"] Received %10u bytes", (uintptr_t)p_conn, ret);
			silc_rng_buf_push(p_conn->rcv_rng, buf, ret);
		}
	}while(ret>0);

	return 0;
}



int silc_tcp_client_conn_setup(silc_socket* p_client, silc_sock_conn* p_conn)
{
	struct hostent host, *p_host = NULL;
	int h_err;
	char tmpbuf[1024];
	silc_socket_type   socket_type;
	socket_type = p_client->opt.socket_type;
	if(socket_type == SILC_SOCKET_TYPE_TCP_CLIENT)
	{
		gethostbyname_r(p_client->opt.o.tcp_client.remote_host, &host, tmpbuf, sizeof(tmpbuf), &p_host, &h_err);
		if(p_host==NULL)
		{
	    	SILC_SOCK_DBG_LOG("[SOCK][%"PRIxPTR"] Failed to resolve %s", (uintptr_t)p_conn, p_client->opt.o.tcp_client.remote_host);
			return -1;
		}

		if(p_host->h_addrtype==AF_INET)
		{
			p_conn->_remote_saddr.sin_family = AF_INET;
			p_conn->_remote_saddr.sin_addr.s_addr = *(in_addr_t*)(p_host->h_addr_list[0]);
			p_conn->_remote_saddr.sin_port = htons(p_client->opt.port_num);
			p_conn->p_remote_saddr = (struct sockaddr*)&p_conn->_remote_saddr;
			p_conn->saddr_len = sizeof(p_conn->_remote_saddr);
		}
		else if(p_host->h_addrtype==AF_INET6)
		{
			p_conn->_remote_saddr6.sin6_family = AF_INET6;
			memcpy(p_conn->_remote_saddr6.sin6_addr.s6_addr, p_host->h_addr_list[0], p_host->h_length);
			p_conn->_remote_saddr6.sin6_port = htons(p_client->opt.port_num);
			p_conn->p_remote_saddr = (struct sockaddr*)&p_conn->_remote_saddr6;
			p_conn->saddr_len = sizeof(p_conn->_remote_saddr);
		}
		return 0;
	}
	else if(socket_type == SILC_SOCKET_TYPE_UNIX_CLIENT)
	{

		p_conn->unix_saddr.sun_family = AF_UNIX;
		strcpy(p_conn->unix_saddr.sun_path,p_client->opt.o.unix_client.unix_domain);
		return 0;
	}
	else
	{
	    SILC_SOCK_DBG_LOG("error client socket type  !error %d \n", p_client->opt.socket_type);
		return -1;
	}

}

int silc_tcp_client_connect(silc_socket* p_client, silc_sock_conn* p_conn)
{
	int ret = 0;
	silc_socket_type   socket_type;
	int stop_flag = 0;
	socket_type = p_client->opt.socket_type;
	if(socket_type == SILC_SOCKET_TYPE_TCP_CLIENT)
	{
		p_conn->sock = socket(AF_INET,SOCK_STREAM, 0);
	}
	else if(socket_type == SILC_SOCKET_TYPE_UNIX_CLIENT)
	{
		p_conn->sock = socket(PF_UNIX,SOCK_STREAM, 0);
	}
	else
	{
		SILC_SOCK_DBG_LOG("error client socket type  !error %d \n", p_client->opt.socket_type);
		return -1;
	}
	if(p_conn->sock==-1)
	{
		SILC_SOCK_DBG_ERR("[SOCK][%"PRIxPTR"] Failed to create socket: %s", (uintptr_t)p_conn, strerror(errno));
		return -1;
	}
	do
	{
		if(socket_type == SILC_SOCKET_TYPE_TCP_CLIENT)
		{
			ret=connect(p_conn->sock, p_conn->p_remote_saddr, p_conn->saddr_len);
			if(p_client->opt.o.tcp_client.reconnect && p_client->running)
			{
				stop_flag = 1;
			}
			else
			{
				stop_flag = 0;
			}

		}
		else if(socket_type == SILC_SOCKET_TYPE_UNIX_CLIENT)
		{
			ret=connect(p_conn->sock, (struct sockaddr*)&p_conn->unix_saddr, sizeof(p_conn->unix_saddr));
			if(p_client->opt.o.unix_client.reconnect && p_client->running)
			{
				stop_flag = 1;
			}
			else
			{
				stop_flag = 0;
			}
		}

		if(ret==0)
		{
			int usr_ret = p_client->opt.callback_conn_new(p_client, p_client->p_usr_data, p_conn);
			if(usr_ret==1)
				return 1;
			if(usr_ret==-1)
			{
				SILC_SOCK_DBG_LOG("[SOCK][%"PRIxPTR"] Caller rejected this connection ", (uintptr_t)p_conn);
				close(p_conn->sock);
				p_conn->sock=-1;
				return -1;
			}
			silc_list_add_tail(&(p_conn->list_node), &(p_client->conn_list));
			FD_SET(p_conn->sock, &(p_client->fdset_backup));
			silc_socket_check_max_fd(p_client);
			p_conn->connected = silc_true;
			return 0;
		}
		else{
			if(socket_type == SILC_SOCKET_TYPE_TCP_CLIENT)
				SILC_SOCK_DBG_LOG("[SOCK][%"PRIxPTR"] Failed to connect to %s(%u) %s", (uintptr_t)p_conn,
						p_client->opt.o.tcp_client.remote_host, p_client->opt.port_num, strerror(errno));
			if(socket_type == SILC_SOCKET_TYPE_UNIX_CLIENT)
				SILC_SOCK_DBG_LOG("[SOCK][%"PRIxPTR"] Failed to connect to %s %s", (uintptr_t)p_conn,
									p_client->opt.o.unix_client.unix_domain, strerror(errno));
			//TODO: this sleep is arbitrary, we should make it configurable
			//second before making another connection attempt
			silc_sem_wait(&p_client->sleep_sem, 100000);
		}
	}
	while(stop_flag);

	return ret;
}

int silc_socket_tcp_client_loop(silc_socket* p_client)
{
	int recv_ret;
	silc_socket_type   socket_type;
	silc_bool quit_on_disconnect = 0;
	socket_type = p_client->opt.socket_type;
	do
	{
		silc_sock_conn* p_conn;
		if(silc_list_empty(&(p_client->conn_list)))
		{//we don't have a silc_sock_conn_t structure yet
			p_conn = silc_sock_conn_create(p_client->opt.max_buf_size, silc_true);
		}
		else
		{
			p_conn = silc_list_entry(p_client->conn_list.next, silc_sock_conn, list_node);
		}
		if(p_conn==NULL)
		{
			return -1;
		}
		if(socket_type == SILC_SOCKET_TYPE_TCP_CLIENT)
		{
			quit_on_disconnect = p_client->opt.o.tcp_client.quit_on_disconnect;
		}
		else if (socket_type == SILC_SOCKET_TYPE_UNIX_CLIENT)
		{
			quit_on_disconnect = p_client->opt.o.unix_client.quit_on_disconnect;
		}
		if(!p_conn->connected)
		{
			int usr_ret;
			silc_tcp_client_conn_setup(p_client, p_conn);

			usr_ret=silc_tcp_client_connect(p_client, p_conn);
			if(usr_ret == 1)
				return 1;
			else if(usr_ret==-1)
			{
				//failed to connect, close the current connection
				silc_sock_conn_close(p_client, p_conn);
				if(quit_on_disconnect)
					break;
				//TODO: this sleep is arbitrary, we should make it configurable
				//second before making another connection attempt
				silc_sem_wait(&p_client->sleep_sem, 100000);
				continue;
			}
		}
		//do a data loop, to do all the communication
		recv_ret=silc_socket_data_recv_loop(p_client);
		if(recv_ret<0)
		{
			silc_sock_conn_close(p_client, p_conn);
		}
		if(recv_ret==1)
		{//user want to interrupt receive, give control back to the caller
			return 1;
		}
	}while(!quit_on_disconnect && p_client->running);

	return 0;
}

int silc_socket_client_close_curr_conn(silc_socket* p_client)
{
	//int usr_ret;
	silc_sock_conn* p_conn;
	if(!silc_list_empty(&(p_client->conn_list)))
	{
		p_conn = silc_list_entry(p_client->conn_list.next, silc_sock_conn, list_node);
		silc_list_del(&p_conn->list_node);
		if(p_client->opt.callback_conn_close)
		{
			p_client->opt.callback_conn_close(p_client, p_client->p_usr_data, p_conn, p_conn->p_usr_data);
		}
	//	usr_ret = silc_sock_conn_close(p_client, p_conn);
	}
	return 0;
}

void silc_sock_conn_set_idle_timeout(silc_socket* p_sock, int timeout_sec)
{
	p_sock->opt.idle_interval.tv_sec = timeout_sec;
}

int silc_sock_conn_get_idle_timeout(silc_socket* p_sock)
{
	return p_sock->opt.idle_interval.tv_sec;
}
