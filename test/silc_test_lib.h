/*
 * silc_test_lib.h
 *
 *  Created on: Apr 6, 2011
 *      Author: jack_chen
 */

#ifndef SILC_TEST_LIB_H_
#define SILC_TEST_LIB_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <popt.h>
#include "silc_common.h"

typedef enum test_lib_arg_type_e{
	TEST_OPT_HELP = 0,
	TEST_ARRAY_FLAG,
	TEST_RBUFFER_FLAG,
	TEST_ELEM_VALUE,
	TEST_ELEM_SIZE,
	TEST_CREATE_SIZE,
	TEST_INPUT_COUNT,
	TEST_ELEMS_SHRINK,

	TEST_FILE_FLAG,
	TEST_FILE_FILE_PATH,
	TEST_FILE_FILE_NAME,
	TEST_FILE_CREATE_DIR,
	TEST_FILE_CREATE_FILE,
	TEST_FILE_CREATE_SIZE,
	TEST_FILE_WRITE_FILE,
	TEST_FILE_WRITE_BUF,
	TEST_FILE_READ_FILE,
	TEST_FILE_MAP_FILE,
	TEST_FILE_MAP_OFF,
	TEST_FILE_MAP_LEN,
	TEST_FILE_FOR_EACH,

	TEST_MEM_FLAG,
	TEST_MEM_ALIGN,
	TEST_MEM_CAP,

	TEST_SOCKET_FLAG,
	TEST_SOCKET_SERVER,
	TEST_SOCKET_CLIENT,
	TEST_SOCKET_UNIX_SERVER,
	TEST_SOCKET_UNIX_CLIENT,
	TEST_SOCKET_UNIX_DOMAIN,
	TEST_SOCKET_IP,
	TEST_SOCKET_PORT,
	TEST_SOCKET_BUF_SIZE,
	TEST_SOCKET_SVR_DUP_IP,
	TEST_SOCKET_SVR_SGLTHRD,
	TEST_SOCKET_CLNT_REMOTEHOST,
	TEST_SOCKET_CLNT_RECONNECT,
	TEST_SOCKET_CLNT_QUDISCONN,
	TEST_SOCKET_CLNT_RBUFF,
	TEST_SOCKET_CLNT_SEND_TIME
}test_lib_arg_type;


typedef struct  test_lib_arg_s
{
	char* elem_value;
	uint32_t elem_size;
	uint32_t create_size;
	uint32_t input_count;
	uint32_t shrink_count;

	char* f_path;
	char* f_name;
	uint64_t cf_size;
	char* write_buf;
	char* mf_length;
    char* mf_offset;

    uint32_t mem_align;
    uint32_t mem_times;

    char* socket_ip;
    uint16_t socket_port;
    uint32_t socket_buf_szie;

    uint32_t allow_duplicate_ip;
    uint32_t svr_sgl_thrd;

    char* clnt_remote_host;
    uint32_t clnt_reconnect;
    uint32_t clnt_qu_on_disconn;
    uint32_t clnt_use_rbuf;
    uint32_t clnt_time_send;

    char* socket_unix_domain;
}test_lib_arg;


#endif /* SILC_TEST_LIB_H_ */
