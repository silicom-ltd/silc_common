/*
 * silc_test_file_lib.c
 *
 *  Created on: Apr 13, 2011
 *      Author: jack_chen
 */

#include "silc_test_file_lib.h"


typedef struct silc_test_file_argv_s
{
	char* f_path;
	char* f_name;
	uint64_t cf_size;
    char* write_buf;
    char* mf_length;
    char* mf_offset;
}silc_test_file_argv;

char* silc_test_file_get_file_path(const char* path1, const char* f_name)
{
	uint32_t len1 = strlen(path1);
	uint32_t len2 = strlen(f_name);
	uint32_t len= len1 + len2 + 2;
	char* tmp_path = (char*)malloc(len);
	snprintf(tmp_path,len, "%s/%s",path1, f_name);
	return tmp_path;
}

void silc_test_file_dir_create(char* path)
{
	if(0 == silc_file_dir_create(path))
	{
		printf("silc_file_dir_create success\n");
	}
	else
	{
		printf("silc_file_dir_create failure\n");
	}
}

void silc_test_file_create_file2(const char* path1, const char* f_name, uint64_t size)
{
	silc_bool fill_zero = silc_false;
	printf("silc_test_file_create_file2 size = %"PRIu64",fill_zero = %d\n",size,fill_zero);
	if(size == 0)
	{
		fill_zero = silc_true;
		printf("silc_test_file_create_file2 size = %"PRIu64",fill_zero = %d\n",size,fill_zero);
	}
	if(0 == silc_file_create_file2((const silc_cstr)path1,(const silc_cstr)f_name,size,fill_zero))
		printf("silc_file_create_file2 success\n");
	else
		printf("silc_file_create_file2 failure\n");
}

void silc_test_file_write_all(const char* path,const char* p_buf)
{
	uint32_t buf_length = strlen(p_buf);
	if(0 == silc_file_write_all(path,p_buf,buf_length))
		printf("silc_file_write_all success\n");
	else
		printf("silc_file_write_all failure\n");
	return;
}

void silc_test_file_read_all(const char* path)
{
	char* pp_ret_data;
	uint32_t p_size;

	if(0 == silc_file_read_all(path,&pp_ret_data,&p_size))
		printf("silc_file_read_all success\n"
				"pp_ret_data: %s \n"
				"p_size = %u \n",pp_ret_data,p_size);
	else
		printf("silc_file_read_all failure\n");
	return;
}

int silc_test_file_dir_for_each_cbf(const silc_cstr path, const silc_cstr fname, const silc_cstr full_path, silc_file_type_t ftype, void* p_arg)
{
     printf("silc_test_file_dir_for_each_cbf path = %s, fname = %s, full_path = %s, ftype = %d\n", path,fname,full_path,ftype);
     return 0;
}

void silc_test_file_dir_for_each(const char* path)
{
	void* p_arg = NULL;
	if(0 == silc_file_dir_for_each((silc_cstr)path,silc_test_file_dir_for_each_cbf,p_arg))
	{
		printf("silc_file_dir_for_each success\n");
	}
	else
	{
		printf("silc_file_dir_for_each failure\n");
	}
}

void silc_test_file_map_file(const char* path,const char* str_off,const char* str_len)
{
	char* end;
	silc_file_map_t p_map;
	int fd = silc_file_open_file((silc_cstr)path);
	uint64_t offset = strtoull(str_off,&end,16);
	if(*end != '\0')
		printf("silc_file_map_file  get offset failed\n");

	uint64_t length = strtoull(str_len,&end,16);
	if(*end != '\0')
		printf("silc_file_map_file  get length failed\n");

	printf("silc_file_map_file  offset = %"PRIx64" , length = %"PRIx64"\n",offset,length);
	if(0 == silc_file_map_file(fd,length,offset,&p_map))
	{
		printf("silc_file_map_file silc_file_map_t usr_length = %"PRIu64", _map_length = %"PRIu64",_p_map = %p\n",//_p_map = %p
				p_map.usr_length,p_map._map_length,p_map._p_map);//*p_map.p_map_usr
		uint64_t i = 0;
		for(; i < p_map.usr_length;i++)
		{
			printf("%c",*((char*)p_map.p_map_usr + i));
		}
		printf("\n");
		printf("silc_file_map_file success\n");
	}
	else
	{
		printf("silc_file_map_file failure\n");
	}
	return;
}

void silc_test_file_access(test_lib_arg* arg,int* code_array,int array_length)
{
	int i = 0;
	char* tmp_path = NULL;
	char* f_path = NULL;
	char* f_name = NULL;
    uint64_t cf_size = 0;
    char* write_buf = NULL;
    char* str_off = NULL;
    char* str_len = NULL;
	if(TEST_FILE_FILE_PATH == code_array[i])
	{
		f_path = strdup(arg->f_path);
		i++;
	}
	else
	{
		printf("the argv f_path is NULL\n");
		return;
	}

	if(TEST_FILE_FILE_NAME == code_array[i])
	{
		f_name = strdup(arg->f_name);
		tmp_path = silc_test_file_get_file_path(f_path,f_name);
		i++;
	}

	switch(code_array[i++])
	{
	    case TEST_FILE_CREATE_DIR:
	    	printf("TEST_FILE_CREATE_DIR\n");
	    	silc_test_file_dir_create(f_path);
	    	break;
	    case TEST_FILE_CREATE_FILE:
	    	printf("TEST_FILE_CREATE_FILE\n");
	    	if(f_name == NULL)
	    	{
	    		printf("silc_file_create_file2 the argv f_name is NULL\n");
	    	}
	    	else
	    	{
	    		if(TEST_FILE_CREATE_SIZE == code_array[i++])
	    			cf_size = arg->cf_size;
	    		silc_test_file_create_file2(f_path,f_name,cf_size);
	    	}
	    	break;
	    case TEST_FILE_WRITE_FILE:
	    	printf("TEST_FILE_WRITE_FILE\n");
	    	if(TEST_FILE_WRITE_BUF == code_array[i++])
	    		write_buf = strdup(arg->write_buf);
	    	if(f_name == NULL || write_buf == NULL)
	    	{
	    		printf("silc_file_write_all the argv f_name is NULL  or write_buf is NULL\n");
	    		break;
	    	}
	    	else
	    	{
	    		silc_test_file_write_all(tmp_path,write_buf);
	    	}
	    	free(write_buf);
	    	break;
        case TEST_FILE_READ_FILE:
        	printf("TEST_FILE_READ_FILE\n");
	    	if(f_name == NULL)
	    	{
	    		printf("silc_file_read_all the argv f_name is NULL\n");
	    	}
	    	else
	    	{
	    		silc_test_file_read_all(tmp_path);
	    	}
	    	break;
	    case TEST_FILE_MAP_FILE:
	    	printf("TEST_FILE_MAP_FILE\n");
	    	if(TEST_FILE_MAP_OFF == code_array[i++] && TEST_FILE_MAP_LEN == code_array[i++])
	    	{
	    		str_off = strdup(arg->mf_offset);
	    		str_len = strdup(arg->mf_length);
	    		silc_test_file_map_file(tmp_path,str_off,str_len);
	    		free(str_off);
	    		free(str_len);
	    	}
	    	else
	    	{
	    		printf("silc_file_write_all the argv mf_offset is NULL  or mf_length is NULL\n");
	    	}
	    	break;
	    case TEST_FILE_FOR_EACH:
	    	printf("TEST_FILE_FOR_EACH\n");
	    	silc_test_file_dir_for_each(f_path);
	    	break;
	    default:
	    	printf("err argv\n");
	}
	free(f_path);
	if(tmp_path != NULL)
		free(tmp_path);
	if(f_name != NULL)
		free(f_name);
	return;

}
