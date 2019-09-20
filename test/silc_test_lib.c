/*
 * silc_test_array_lib.c
 *
 *  Created on: Apr 1, 2011
 *      Author: Jack_chen
 *   Copyright: NetPerform Technology
 */
#include "silc_test_lib.h"
#include "silc_test_array_lib.h"
#include "silc_test_file_lib.h"
#include "silc_test_ringbuf_lib.h"
#include "silc_test_mem_lib.h"
#include "silc_test_socket_lib.h"

#define TEST_MAX_CODE 32



int main(int argc,const char* argv[])
{
	test_lib_arg arg;
	memset(&arg,0,sizeof(arg));
	struct poptOption option_table[] =
	{
		{ "array", 'A', POPT_ARG_NONE, NULL, TEST_ARRAY_FLAG,            "test array lib", NULL },
		{ "ring-buffer", 'B', POPT_ARG_NONE, NULL, TEST_RBUFFER_FLAG,          "test ring buffer lib", NULL },
		{ "memory", 'M', POPT_ARG_NONE, NULL, TEST_MEM_FLAG,          "test memory lib", NULL },
		{ "file",               'F', POPT_ARG_NONE,   NULL,TEST_FILE_FLAG, 		                "test file lib", NULL },
		{ "socket",               'S', POPT_ARG_NONE,   NULL,TEST_SOCKET_FLAG, 		                "test socket lib", NULL },

		{ "alignment",'a', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT, &arg.mem_align,TEST_MEM_ALIGN,          "memory alignment", NULL },

		{ "value", 'v', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT, &arg.elem_value,TEST_ELEM_VALUE, "value of element", NULL },
		{ "size", 'y', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT, &arg.elem_size,TEST_ELEM_SIZE, "size of element", NULL },

		{ "create",'c', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT, &arg.create_size,TEST_CREATE_SIZE,          "create  size", NULL },
		{ "input", 'i', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT, &arg.input_count,TEST_INPUT_COUNT,            "input element count", NULL },
		{ "shrink",'s', POPT_ARG_INT, &arg.shrink_count,TEST_ELEMS_SHRINK, "shrink array", NULL },
		{ "times",'t', POPT_ARG_INT, &arg.mem_times,TEST_MEM_CAP,          "test times", NULL },

		{ "path",               'p', POPT_ARG_STRING, &arg.f_path,TEST_FILE_FILE_PATH, 		"file path", NULL },
		{ "name",               'n', POPT_ARG_STRING, &arg.f_name,TEST_FILE_FILE_NAME, 		"file name", NULL },
		{ "create-directory",   'd', POPT_ARG_NONE,   NULL,TEST_FILE_CREATE_DIR, 	                "test create directory feature", NULL },
		{ "create-file",        'f', POPT_ARG_NONE,   NULL,TEST_FILE_CREATE_FILE, 		            "test create file feature", NULL },
		{ "size",               'z', POPT_ARG_LONG,   &arg.cf_size,TEST_FILE_CREATE_SIZE,   	"size of create file", NULL },
		{ "write-file",         'w', POPT_ARG_NONE,   NULL,TEST_FILE_WRITE_FILE, 	            	"test write file feature", NULL },
		{ "buffer",             'b', POPT_ARG_STRING, &arg.write_buf,TEST_FILE_WRITE_BUF,      "write file buffer", NULL },
		{ "read-file",          'r', POPT_ARG_NONE,   NULL,TEST_FILE_READ_FILE, 		            "test read file feature", NULL },
		{ "map-file",           'm', POPT_ARG_NONE,   NULL,TEST_FILE_MAP_FILE, 		                "test map file feature", NULL },
		{ "offset",             'o', POPT_ARG_STRING, &arg.mf_offset,TEST_FILE_MAP_OFF, 		"offset of start map file", NULL },
		{ "length",             'l', POPT_ARG_STRING, &arg.mf_length,TEST_FILE_MAP_LEN, 		"length of map file", NULL },
		{ "each",               'e', POPT_ARG_NONE,   NULL,TEST_FILE_FOR_EACH, 		                "action foe each file", NULL },


		{ "socket-server",          0, POPT_ARG_NONE,   NULL,TEST_SOCKET_SERVER, 		            "test socket lib server", NULL },
		{ "socket-client",          0, POPT_ARG_NONE,   NULL,TEST_SOCKET_CLIENT, 		            "test socket lib client", NULL },
		{ "socket-ip",              0, POPT_ARG_STRING, &arg.socket_ip,TEST_SOCKET_IP, 		        "test socket lib,input ip", NULL },
		{ "socket-port",            0, POPT_ARG_INT,    &arg.socket_port,TEST_SOCKET_PORT, 		    "test socket lib,input port", NULL },
		{ "socket-bufsize",         0, POPT_ARG_INT,    &arg.socket_buf_szie,TEST_SOCKET_BUF_SIZE, 	"test socket lib,input max buffer size", NULL },

		{ "socket-unix-server",          0, POPT_ARG_NONE,   NULL,TEST_SOCKET_UNIX_SERVER, 		     "test socket lib unix server", NULL },
		{ "socket-unix-client",          0, POPT_ARG_NONE,   NULL,TEST_SOCKET_UNIX_CLIENT, 		     "test socket lib unix client", NULL },
		{ "socket-unix-domain",  0, POPT_ARG_STRING, &arg.socket_unix_domain,TEST_SOCKET_UNIX_DOMAIN, "test socket lib,input unix domain", NULL },

		{ "s-dupip",           0, POPT_ARG_INT,    &arg.allow_duplicate_ip,TEST_SOCKET_SVR_DUP_IP, 	"test socket lib,input 0 declare don't allow duplicate ip", NULL },
		{ "s-sglthrd",         0, POPT_ARG_INT,    &arg.svr_sgl_thrd,TEST_SOCKET_SVR_SGLTHRD,  "test socket lib,input 0 declare don't single thread", NULL },

		{ "c-remotehost",      0, POPT_ARG_INT,    &arg.clnt_remote_host,TEST_SOCKET_CLNT_REMOTEHOST,  "test socket lib,remote host", NULL },
		{ "c-reconnect",       0, POPT_ARG_INT,    &arg.clnt_reconnect,TEST_SOCKET_CLNT_RECONNECT,   "test socket lib,input 0 declare don't reconnect", NULL },
		{ "c-quonconn",        0, POPT_ARG_INT,    &arg.clnt_qu_on_disconn,TEST_SOCKET_CLNT_QUDISCONN,   "test socket lib,input 0 declare don't quit on disconnect", NULL },
		{ "c-userbuff",        0, POPT_ARG_INT,    &arg.clnt_use_rbuf,TEST_SOCKET_CLNT_RBUFF,       "test socket lib,input 0 declare don't use ring buffer", NULL },
		{ "c-sendtime",        0, POPT_ARG_INT,    &arg.clnt_time_send,TEST_SOCKET_CLNT_SEND_TIME,       "test socket lib,second", NULL },
		POPT_AUTOHELP
		POPT_TABLEEND
	};
	poptContext option_context = poptGetContext(argv[0], argc, argv,option_table, 0);

	//poptContext poptGetContext(const char * name,int argc,const char ** argv,const struct poptOption * options,unsigned int flags);
	if (argc < 2)
	{
		poptPrintUsage(option_context, stderr, 0);
		exit(-1);
	}

	int test_type = poptGetNextOpt(option_context);
	int code_array[TEST_MAX_CODE];
	int i = TEST_MAX_CODE - 1;
	for(;i >= 0;i--)
	{
		code_array[i] = -1;
	}

	int code;
	i = 0;
	while ((code = poptGetNextOpt(option_context)) >= 0)
	{
		code_array[i] = code;
		printf("code =%d,i = %d  ",code_array[i],i);
		i++;
		if(i == TEST_MAX_CODE - 1)
			break;
	}
	printf("\n");
	switch (test_type)
	{
	    case TEST_ARRAY_FLAG:
	    	printf("silc_common test array  test_type = %d,value =%s ,create = %d, input = %d,shrink = %d\n",
	    			test_type,arg.elem_value,arg.create_size,arg.input_count,arg.shrink_count);
		    silc_test_array_access(&arg,code_array,TEST_MAX_CODE);
		    break;
	    case TEST_FILE_FLAG:
	    	printf("silc_common test file  test_type = %d, f_path = %s, f_name = %s ,cf_size = %"PRIu64", mf_offset = %s, mf_length = %s, write_buf = %s\n",
	    		    			test_type,arg.f_path,arg.f_name,arg.cf_size,arg.mf_offset,arg.mf_length,arg.write_buf);
	    	silc_test_file_access(&arg,code_array,TEST_MAX_CODE);
	    	break;
	    case TEST_RBUFFER_FLAG:
	    	printf("silc_common test ring buffer test_type = %d,elem_value = %s ,create_size = %d\n",
	    	    			    test_type,arg.elem_value,arg.create_size);
	    	silc_test_ring_buffer_access(&arg,code_array,TEST_MAX_CODE);
	    	break;
	    case TEST_MEM_FLAG:
	    	printf("silc_common test memory test_type = %d,elem_value = %s ,create_size = %u, mem_align = %u\n",
	    		    	    			    test_type,arg.elem_value,arg.create_size,arg.mem_align);
	    	silc_test_memory_access(&arg,code_array,TEST_MAX_CODE);
	    	break;
	    case TEST_SOCKET_FLAG:
	    	printf("silc_common test memory test_type = %d,elem_value = %s ,create_size = %u, mem_align = %u\n",
	    	    		    	    			    test_type,arg.elem_value,arg.create_size,arg.mem_align);
	    	silc_test_socket_access(&arg,code_array,TEST_MAX_CODE);
	    	break;
		default:
			poptPrintUsage(option_context, stderr, 0);
			exit(-1);
	}
	/*if (code < -1)
	{
		fprintf(stderr, "%s: %s\n",poptBadOption(option_context, POPT_BADOPTION_NOALIAS),
				poptStrerror(code));
		return -1;
	}*/
	if(NULL == poptFreeContext(option_context))
		printf("free option_context \n");
	if(arg.elem_value != NULL)
		free(arg.elem_value);
	if(arg.f_path != NULL)
		free(arg.f_path);
	if(arg.f_name != NULL)
		free(arg.f_name);
	if(arg.write_buf != NULL)
		free(arg.write_buf);
	if(arg.mf_offset != NULL)
		free(arg.mf_offset);
	if(arg.mf_length != NULL)
		free(arg.mf_length);
	return 0;
}

#ifdef  SILC_LOG_SMRLOG
int
lc_log_internal(const char *function, const char *file, uint32 line,
                uint32 log_opts, uint32 msg_tags, int level,
                tbool skip_lock, const char *format, ...)
{
	va_list ap;
	va_start(ap,format);
	int ret = vfprintf(stderr, format, ap);
	va_end(ap);
	return ret;
}

#endif
