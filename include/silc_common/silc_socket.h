/*
 * silc_socket.h
 *
 *  Created on: Nov 22, 2010
 *      Author: jeff_zheng
 *   Copyright: NetPerform Technology
 */

#ifndef SILC_SOCKET_H_
#define SILC_SOCKET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <arpa/inet.h>
/**
 * @defgroup silc_socket silc_socket
 * @brief A socket wrapper
 * @ingroup silc_common
 *
 * A socket wrapper, can be used as tcp server of client.
 */

typedef struct silc_socket_s silc_socket;

typedef struct silc_sock_conn_s silc_sock_conn, *silc_sock_conn_id;

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
typedef int (*silc_socket_callback_conn_new)(silc_socket* p_socket, void* p_svr_arg, silc_sock_conn_id p_conn);
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
typedef int (*silc_socket_callback_conn_close)(silc_socket* p_socket, void* p_svr_arg, silc_sock_conn_id p_conn, void* p_conn_arg);


/**
 * @ingroup silc_socket
 * @brief call back when a connection is idle for the configured time
 *
 * @param[in] p_socket	the socket on which the connection is received
 * @param[in] p_usr_arg the p_usr_arg passed to silc_socket_create
 * @param[in] p_conn    the new connection
 * @param[in] p_conn_arg    user argument of this conneciton
 * @return 0 to accept the connection. other value to reject the connection
 *
 * the infrastructure will call this callback to notify the user that the connection has been idle more than the configured time.
 * This function is mostly useful for breaking out receive loop function by returning 1;
 * return 0 if the user want nothing to happen
 * return -1 if the user want to shutdown the connection
 **/
typedef int (*silc_socket_callback_conn_idle)(silc_socket* p_socket, void* p_svr_arg, silc_sock_conn_id p_conn, void* p_conn_arg);

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
typedef int (*silc_socket_callback_data_read)( silc_socket* p_socket, void* p_svr_arg, silc_sock_conn_id p_conn, void* p_conn_arg, silc_rng_buf_id p_data_rng_buf);


typedef struct silc_socket_svr_opt_s
{
    silc_bool 	allow_duplicate_ip;		//allow multiple connections from the same remote address
    silc_bool 	single_thread;			//single thread model, data receive and processing will all be in a single thread,
    									// this will require any data processing callback to be non-blocking.
    									//Thus only recommended for single applications
    char		bind_address[256];		//server bind address
}silc_socket_opt_tcp_svr;

typedef struct silc_tcp_client_opt_s
{
	char	 	remote_host[256];		//address of the server to connect to
    silc_bool		reconnect;				//reconnect on disconnect
	silc_bool		quit_on_disconnect;		//quit when disconnect
}silc_socket_opt_tcp_client;

typedef struct silc_unix_svr_opt_s
{
    silc_bool 	allow_duplicate_ip;		//allow multiple connections from the same remote address
    silc_bool 	single_thread;			//single thread model, data receive and processing will all be in a single thread,
    									// this will require any data processing callback to be non-blocking.
    									//Thus only recommended for single applications
    char		unix_domain[256];
}silc_socket_opt_unix_svr;

typedef struct silc_unix_client_opt_s
{
	char	 	unix_domain[256];
    silc_bool		reconnect;				//reconnect on disconnect
	silc_bool		quit_on_disconnect;		//quit when disconnect
}silc_socket_opt_unix_client;

typedef enum silc_socket_type_e
{
	SILC_SOCKET_TYPE_TCP_SERVER   = 1,
	SILC_SOCKET_TYPE_TCP_CLIENT   = 2,
	SILC_SOCKET_TYPE_UNIX_SERVER   = 3,
	SILC_SOCKET_TYPE_UNIX_CLIENT   = 4,
}silc_socket_type;


typedef struct silc_socket_opt_s
{
	silc_socket_type    	socket_type;			//server of client,
	uint32_t 			max_buf_size;			//maximum buffer size, this should typically be at least larger then the largest message
	uint16_t 			port_num;				//port number, for client, this is the server port number to connect to.
												//for server, this is the listen port
	struct timeval		idle_interval;			//minimum interval between idle callback
	silc_socket_callback_conn_new 	callback_conn_new;	//callback for new connection setup
    silc_socket_callback_data_read 	callback_data_read;	//callback for data processing
    silc_socket_callback_conn_close 	callback_conn_close;//callback for connection shutdwon
    silc_socket_callback_conn_idle	callback_conn_idle; //callback for connection idle

    //options for the socket
    union
    {
    	silc_socket_opt_tcp_svr tcp_svr;
    	silc_socket_opt_tcp_client tcp_client;
    	silc_socket_opt_unix_svr unix_svr;
    	silc_socket_opt_unix_client unix_client;
    }o;

}silc_socket_opt;

/**
 * @ingroup silc_socket
 * @brief return the pointer to the configuration of the socket.
 *
 * @param[in] p_sock		the socket
 * @return the scoket opt pointer
 *
 * note the returned opt pointer points to a copy of the opt structure, and there might be difference
 * between the opt given to silc_socket_create and this returned opt. some of the values may be initialized
 * to default values. for example the idle_interval.
 **/
extern const silc_socket_opt* silc_socket_get_opt(silc_socket* p_sock);

/**
 * @ingroup silc_socket
 * @brief create a new socket
 *
 * @param[in] opts				socket options
 * @param[in] p_usr_data		tranparent use data pointer to be passed to callback functions
 * @return the created socket object, NULL for Error
 *
 **/
extern silc_socket* silc_socket_create(silc_socket_opt* opts, void* p_usr_data);
extern void silc_socket_stop(silc_socket* p_socket);
extern void silc_socket_destroy(silc_socket* p_socket);
/**
 * @ingroup silc_socket
 * @brief main data receive loop for server type socket
 *
 * @param[in] silc_socket			the socket
 * @return 0 for normal quit \n 1 when user what to interrupt \n -1 for error
 *
 * User should call this function to start receive connections on the socket.
 * This is a blocking function, and will only return
 * 1 when user callback returns 1, which means the caller want to interrupt
 * 0 for graceful shutdown
 * -1 for error returns
 **/
extern int silc_socket_data_recv_loop(silc_socket* p_svr);
/**
 * @ingroup silc_socket
 * @brief main data receive loop for client type socket
 *
 * @param[in] silc_socket			the socket
 * @return 0 for normal quit \n 1 when user what to interrupt \n -1 for error
 *
 * User should call this function to start connect to the remote server and receive data on the socket.
 * This is a blocking function, and will only return
 * 1 when user callback returns 1, which means the caller want to interrupt
 * 0 for graceful shutdown
 * -1 for error returns
 **/
extern int silc_socket_tcp_client_loop(silc_socket* p_client);

/**
 * @ingroup silc_socket
 * @brief to close the current connection in a client socket
 *
 * @param[in] silc_socket			the socket
 * @return 0 for normal quit \n -1 for error
 *
 * call this function to close the current connection. The user may do this to start a new connection
 * in case of a protocol error.
 **/
extern int silc_socket_client_close_curr_conn(silc_socket* p_client);

/**
 * @ingroup silc_socket
 * @brief set the user arg on an connection
 *
 * @param[in] p_conn  the connection
 * @param[in] p_usr_arg the user arg to set
 * @return 0 for normal quit \n -1 for error
 *
 * set an transparent user arg for the data receive callback.
 * normally the user call this function in the callback_conn_new callback.
 **/
extern void silc_sock_conn_set_arg(silc_sock_conn* p_conn, void* p_usr_arg);

/**
 * @ingroup silc_socket
 * @brief return the underlying socket fd of an connection
 *
 * @param[in] p_conn  the connection
 * @return the socket fd
 *
 **/
extern int silc_sock_conn_get_sock(silc_sock_conn* p_conn);

/**
 * @ingroup silc_socket
 * @brief send data to the remote side of the connection
 *
 * @param[in] p_conn  the connection
 * @param[in] buffer  the pointer to the start of the data
 * @param[in] size    size of the data
 * @return 0 for normal quit \n -1 for error
 *
 **/
extern int silc_sock_conn_send(silc_sock_conn* p_conn, void* buffer, uint32_t size);

/**
 * @ingroup silc_socket
 * @brief close the socket connection
 *
 * @param[in] p_sock  the socket
 * @param[in] p_conn  the connection
 * @return 0 for normal quit \n -1 for error
 *
 **/
extern int silc_sock_conn_close(silc_socket* p_socket, silc_sock_conn* p_conn);

/**
 * @ingroup silc_socket
 * @brief set idle timeout of the connection
 *
 * @param[in] p_sock  the socket
 * @param[in] timeout_sec timeout of seconds
 *
 **/
extern void silc_sock_conn_set_idle_timeout(silc_socket* p_sock, int timeout_sec);

/**
 * @ingroup silc_socket
 * @brief get idle timeout of the connection
 *
 * @param[in] p_sock  the socket
 * @return timeout number
 *
 **/
extern int silc_sock_conn_get_idle_timeout(silc_socket* p_sock);


#ifdef __cplusplus
}
#endif

#endif /* SILC_SOCKET_H_ */
