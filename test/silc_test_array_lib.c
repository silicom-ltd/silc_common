/*
 * silc_test_array_lib.c
 *
 *  Created on: Apr 13, 2011
 *      Author: jack_chen
 */
#include "silc_test_array_lib.h"


typedef struct  test_array_arg_s
{
	uint32_t elem_size;
	char* elem_value;
	uint32_t create;
	uint32_t input;
	uint32_t shrink;
}test_array_arg;

void silc_test_array_access(test_lib_arg* arg,int* code_array,int array_length)
{
	test_array_arg data = {0,NULL,0,0,0};
	silc_bool is_create = silc_false;
	silc_bool is_set_value = silc_false;
	int i = 0;
	silc_array* silc_test_array = NULL;
	while(code_array[i] >= 0)
	{
		switch (code_array[i])
		{
		    case TEST_ELEM_VALUE:
		    	data.elem_value = strdup(arg->elem_value);
		    	if(data.elem_value == NULL || 0 == strlen(data.elem_value))
		    	{
		    		data.elem_value = "123456789";
		    	}
		    	data.elem_size = strlen(data.elem_value) + 1;
		    	printf("data.elem_value = %s,data.elem_size = %u\n",data.elem_value,data.elem_size);
		    	is_set_value = silc_true;
		    	break;
		    case TEST_CREATE_SIZE:
		    	if(is_set_value)
		    	{
		    		data.create = arg->create_size;
		        	silc_test_array = silc_array_create(data.elem_size, data.create);
		        	if(silc_test_array == NULL)
		        		exit(-1);
		    	    is_create = silc_true;
		    	    printf("silc_array_create\n");
		    	}
		    	else
		    	{
		    		exit(-1);
		    	}
				break;
			case TEST_INPUT_COUNT:
                if(is_create == silc_true)
                {
                	data.input = arg->input_count;
                	if(data.input == 0)
                	{
                		data.input = 10;
                	}
                	char* elem_set = (char*)malloc(data.elem_size * data.input);
                	uint16_t j = 0;
                	for(; j < data.input; j++)
                	{
                      	memcpy(elem_set + j*data.elem_size,data.elem_value,data.elem_size);
                      	printf("elem_set  %s\n",(elem_set + j*data.elem_size));
                	}
                	silc_array_put_tail(silc_test_array, elem_set, data.input);
                	j = 0;
                	for(; j < silc_test_array->length; j++)
                	{
                		char* tmp_elem = silc_array_get_quick(silc_test_array,j);
                	    if(0 == (j % silc_test_array->blk_elem_cnt))
                	    printf("\n");
                	    printf("%-10s",tmp_elem);
                	}
                	printf(" ss\n");
                	silc_array_put_tail(silc_test_array, elem_set, data.input);
                	                	j = 0;
                	                	for(; j < silc_test_array->length; j++)
                	                	{
                	                		char* tmp_elem = silc_array_get_quick(silc_test_array,j);
                	                	    if(0 == (j % silc_test_array->blk_elem_cnt))
                	                	    printf("\n");
                	                	    printf("%-10s",tmp_elem);
                	                	}
                	free(elem_set);
                }
                else
                {
                	exit(-1);
                }
				break;
			case TEST_ELEMS_SHRINK:
				data.shrink = arg->shrink_count;
				if(data.shrink < silc_test_array->length)
					silc_array_shrink(silc_test_array, data.shrink);
				uint16_t i = 0;
				for(; i < silc_test_array->length; i++)
				{
				     char* tmp_elem = silc_array_get_quick(silc_test_array,i);
				                		if(0 == (i % silc_test_array->blk_elem_cnt))
				                			printf("\n");
				                		printf("%-10s",tmp_elem);
				}
				printf("\n");
				break;
		}
		i++;
	}
}

