/*
 * silc_test_ringbuf_lib.c
 *
 *  Created on: Apr 13, 2011
 *      Author: jack_chen
 */

#include "silc_test_ringbuf_lib.h"

typedef enum silc_test_rbuf_action_e
{
	SILC_TEST_RBUF_NONE = 0,
	SILC_TEST_RBUF_PUSH = 1,
	SILC_TEST_RBUF_PAEK = 2,
	SILC_TEST_RBUF_FPUSH = 3,
	SILC_TEST_RBUF_POP = 4,
	SILC_TEST_RBUF_CLEAR = 5,
	SILC_TEST_RBUF_EXIT
}silc_test_rbuf_action;

void silc_test_rng_dtor_cbf(void* p_elem)
{

}

void silc_test_rng_buf_push(silc_rng_buf_id r_buf,uint32_t input_count,char* elem_value,uint32_t elem_size)
{
	char* elem_set = (char*)malloc(elem_size * input_count);
	uint32_t j = 0;
	for(; j < input_count; j++)
	{
		memcpy(elem_set + j * elem_size,elem_value,elem_size);
		printf("%s  ",(char*)(elem_set + j * elem_size));
	}
	printf("\n");
	if(0 == silc_rng_buf_push(r_buf,(uint8_t*)elem_set,input_count))
	{
		printf("silc_test_rng_buf_push   p_buf = %p, idx_first = %u, idx_last = %u, total_count = %u, elem_size = %u\n",
				r_buf->p_buf,r_buf->idx_first,r_buf->idx_last,r_buf->total_count,r_buf->elem_size);
		uint32_t idx_first = r_buf->idx_first;
		while(idx_first != r_buf->idx_last)
		{
			printf("%s ",(char*)(r_buf->p_buf + idx_first * r_buf->elem_size));
			idx_first++;
			if(idx_first == r_buf->total_count)
				idx_first = 0;
		}
		printf("\n");
		printf("silc_test_rng_buf_push success\n");
	}
	else
	{
		printf("silc_test_rng_buf_push failure\n");
	}
	return;
}

void silc_test_rng_buf_peak(silc_rng_buf_id r_buf, uint32_t start_offset,uint32_t read_cnt)
{
	char* p_usr_buf = (char*)malloc(r_buf->elem_size * read_cnt);
	memset(p_usr_buf,0,r_buf->elem_size * read_cnt);
	if(0 == silc_rng_buf_peak(r_buf,start_offset,p_usr_buf,read_cnt))
	{
		printf("silc_test_rng_buf_peak   p_buf = %p, idx_first = %u, idx_last = %u, total_count = %u, elem_size = %u\n",
						r_buf->p_buf,r_buf->idx_first,r_buf->idx_last,r_buf->total_count,r_buf->elem_size);
		uint32_t i = 0;
		for(;i < read_cnt;i++)
		{
			printf("%s ",p_usr_buf + i * r_buf->elem_size);
		}
		printf("silc_test_rng_buf_peak success\n");
	}
	else
	{
		printf("silc_test_rng_buf_peak failure\n");
	}
	return;
}

void silc_test_rng_buf_push_force(silc_rng_buf_id r_buf)
{
	char* p_push_data = (char*)malloc(r_buf->elem_size);
	char* p_pop_buf = (char*)malloc(r_buf->elem_size);
	uint32_t i = 0;
	char a = 'a';
	for(;i < r_buf->elem_size - 1;i++)
	{
		*(p_push_data + i) = a + i;
	}
	*(p_push_data + r_buf->elem_size - 1) = '\0';
	printf("silc_test_rng_buf_push_force p_push_data: %s \n",p_push_data);
	if(0 == silc_rng_buf_push_force(r_buf,p_push_data,p_pop_buf))
	{
		printf("silc_test_rng_buf_peak   p_buf = %p, idx_first = %u, idx_last = %u, total_count = %u, elem_size = %u\n",
								r_buf->p_buf,r_buf->idx_first,r_buf->idx_last,r_buf->total_count,r_buf->elem_size);
		uint32_t idx_first = r_buf->idx_first;
		while(idx_first != r_buf->idx_last)
	    {
			printf("%s ",(char*)(r_buf->p_buf + idx_first * r_buf->elem_size));
			idx_first++;
			if(idx_first >= r_buf->total_count)
			{
				idx_first = 0;
			}
		}
		printf("\n");
		printf("silc_test_rng_buf_push_force p_pop_buf: %s \n",p_pop_buf);
		printf("silc_test_rng_buf_push_force success\n");
	}
	else
	{
		printf("silc_test_rng_buf_push_force failure\n");
	}
	return;
}

void silc_test_rng_buf_clear(silc_rng_buf_id r_buf)
{
	silc_rng_buf_clear(r_buf);
	printf("silc_test_rng_buf_clear   p_buf = %p, idx_first = %u, idx_last = %u, total_count = %u, elem_size = %u\n",
						r_buf->p_buf,r_buf->idx_first,r_buf->idx_last,r_buf->total_count,r_buf->elem_size);
	printf("silc_test_rng_buf_clear success\n");
	return;
}

void silc_test_rng_buf_pop(silc_rng_buf_id r_buf,uint32_t pop_count)
{
	char* p_pop_buf = (char*)malloc(r_buf->elem_size * pop_count);
	memset(p_pop_buf,0,r_buf->elem_size * pop_count);

	printf("\n");
	if(0 == silc_rng_buf_pop(r_buf,p_pop_buf,pop_count))
	{
		printf("silc_test_rng_buf_pop   p_buf = %p, idx_first = %u, idx_last = %u, total_count = %u, elem_size = %u\n",
				r_buf->p_buf,r_buf->idx_first,r_buf->idx_last,r_buf->total_count,r_buf->elem_size);
		uint32_t j = 0;
		for(; j < pop_count; j++)
		{
			printf("%s  ",p_pop_buf + j * r_buf->elem_size);
		}
		printf("\n");
		printf("silc_test_rng_buf_pop success\n");
	}
	else
	{
		printf("silc_test_rng_buf_pop failure\n");
	}
	return;
}

void silc_test_ring_buffer_access(test_lib_arg* arg,int* code_array,int array_length)
{
	silc_bool is_set_value = silc_false;
	int i = 0;
	char* elem_value = NULL;
	uint32_t elem_size = 0;
	uint32_t create_size = 0;

	silc_rng_buf_id r_buf = NULL;
	while(code_array[i] >= 0)
	{
		switch (code_array[i])
		{
		    case TEST_ELEM_VALUE:
		    	elem_value = strdup(arg->elem_value);
		    	if(elem_value == NULL || 0 == strlen(elem_value))
		    	{
		    		break;
		    	}
		    	elem_size = strlen(elem_value) + 1;
		    	printf("elem_value = %s,elem_size = %u\n",elem_value,elem_size);
		    	is_set_value = silc_true;
		    	break;
		    case TEST_CREATE_SIZE:
		    	if(!is_set_value)
		    	{
		    		elem_value = "123456789";
		    		elem_size = strlen(elem_value) + 1;
		    		printf("elem_value = %s,elem_size = %u\n",elem_value,elem_size);
		    	}
		    	create_size = arg->create_size;
		    	r_buf = silc_rng_buf_create(elem_size,create_size,silc_test_rng_dtor_cbf);
		    	if(r_buf == NULL)
		    	{
		    		printf("silc_rng_buf_create failed\n");
		    		return;
		    	}
		    	printf("silc_array_create success,idx_first = %u,idx_last = %u,total_count = %u\n",r_buf->idx_first,r_buf->idx_last,r_buf->total_count);
				break;
		    default:
		    	printf("err argv\n");
		    	return;
		}
		i++;
	}
	int select;
	while(1)
	{
		//input_count = rand() % create_size;
		printf("select:\n"
			   "%d:push ring buffer action\n"   //silc_rng_buf_push(silc_rng_buf_id rng_id, uint8_t const *p_element, uint32_t count)
			   "%d:peak elem action\n"   //silc_rng_buf_peak(silc_rng_buf_id rng_id, uint32_t start_offset, void* p_usr_buf, uint32_t read_cnt)
			   "%d:force push ring buffer action \n"  //int silc_rng_buf_push_force(silc_rng_buf_id rng_id, const void* p_push_data,  void* p_pop_buf)
			   "%d:pop ring buffer action\n"         //int silc_rng_buf_pop(silc_rng_buf_id rng_id, void* p_pop_buf, uint32_t read_cnt)
			   "%d:clear ring buffer action\n"        //silc_rng_buf_clear(silc_rng_buf_id rng_id)
			   "%d:exit\n",
			   SILC_TEST_RBUF_PUSH,
			   SILC_TEST_RBUF_PAEK,
			   SILC_TEST_RBUF_FPUSH,
			   SILC_TEST_RBUF_POP,
			   SILC_TEST_RBUF_CLEAR,
			   SILC_TEST_RBUF_EXIT);
		printf("please input:");
		if(1 != scanf("%d",&select))
		{
			printf("input select error\n");
			return;
		}
		if(select <= SILC_TEST_RBUF_NONE || select > SILC_TEST_RBUF_EXIT)
		{
			printf("input select error,please again\n");
			continue;
		}

		uint32_t input_count;
		uint32_t start_offset;
		uint32_t peak_count;
		uint32_t pop_count;

		switch (select)
		{
		    case SILC_TEST_RBUF_PUSH:
		    	printf("please input push count:");
		    	if(1 != scanf("%u",&input_count))
		    	{
		    		printf("input push count error\n");
		    		return;
		    	}
		    	silc_test_rng_buf_push(r_buf,input_count,elem_value,elem_size);
				break;
			case SILC_TEST_RBUF_PAEK:
				printf("please input peak start offset:");
				if(1 != scanf("%u",&start_offset))
				{
					printf("input peak start offset error\n");
					return;
				}
				printf("please input peak count:");
				if(1 != scanf("%u",&peak_count))
				{
					printf("input peak count error\n");
					return;
				}
				silc_test_rng_buf_peak(r_buf,start_offset,peak_count);
				break;
			case SILC_TEST_RBUF_FPUSH:
				silc_test_rng_buf_push_force(r_buf);
				break;
			case SILC_TEST_RBUF_POP:
				printf("please input pop count:");
				if(1 != scanf("%u",&pop_count))
				{
					printf("input pop count error\n");
					return;
				}
				silc_test_rng_buf_pop(r_buf,pop_count);
				break;
			case SILC_TEST_RBUF_CLEAR:
				silc_test_rng_buf_clear(r_buf);
				break;
			case SILC_TEST_RBUF_EXIT:
			default:
				return;
		}
	}
}
