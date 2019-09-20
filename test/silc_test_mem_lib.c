/*
 * silc_test_mem_lib.c
 *
 *  Created on: Apr 13, 2011
 *      Author: jack_chen
 */

#include "silc_test_mem_lib.h"

void silc_test_memory_cap(silc_mcache* mem_cache,uint32_t times)
{
	uint32_t i;
	struct timeval last_time;
	struct timeval curr_time;
	silc_mcache_blk* blk = silc_list_entry(mem_cache->mblk_free_list.next, silc_mcache_blk, usage_node);
	uint32_t all_size = 10000000;
	uint32_t free_size = all_size >> 1;

	int** free_array = (int**)malloc(times * sizeof(int*));
	int* bit_flag = (int*)malloc(all_size * sizeof(int));

	for(i = 0;i < times;i++)
	{
		free_array[i] = (int*)malloc(free_size * sizeof(int));
	}
	printf("free_size = %u, all_size = %u\n",free_size,all_size);

	uint32_t j;
    for(i = 0;i < times;i++)
    {
    	memset(bit_flag,0,all_size * sizeof(int));
    	for(j= 0; j < free_size;j++)
    	{
    		while(1)
    		{
    			uint32_t value = rand() % all_size;
       //		printf("value = %u, i = %u\n",value,i);
    	      	if(bit_flag[value] == 0)
    	        {
    	      		free_array[i][j] = value;
    			    bit_flag[value] = 1;
    			    break;
    	        }
    	    }
    	}
    }
    //silc_slist_node* node = blk->free_list.first;
     printf("set free_array  end\n");

	char** elem_arry = (char**)malloc(all_size * sizeof(char*));

	for(i = 0;i < all_size;i++)
	{
		elem_arry[i] = silc_mcache_alloc_real(mem_cache);
		if(elem_arry[i] == NULL)
		{
			printf("get elem_arr error,i = %u\n",i);
			return;
		}
	}

	printf("set silc_mcache_alloc_real  end,mblk_cnt = %u\n",mem_cache->mblk_cnt);
	for(j = 0;j < times;j++)
	{
		gettimeofday(&last_time, NULL);
		for(i = 0;i < free_size;i++)
		{
			if(0 != silc_mcache_free_real(elem_arry[free_array[j][i]]))
			{
				printf("silc_mcache_free_real return err,i = %u",i);
				return;
			}
		}
		for(i = 0;i < free_size;i++)
		{
			elem_arry[free_array[j][i]] = silc_mcache_alloc_real(mem_cache);
			if(elem_arry[free_array[j][i]] == NULL)
			{
				printf("get elem_arr error,i = %u\n",i);
				return;
			}
		}
		gettimeofday(&curr_time, NULL);
		if(curr_time.tv_usec > last_time.tv_usec)
		{
			printf("silc_mcache_add_new_mblk time = %ld.%ld\n",
					curr_time.tv_sec - last_time.tv_sec,curr_time.tv_usec - last_time.tv_usec);
		}
		else
		{
			printf("silc_mcache_add_new_mblk time = %ld.%ld\n",
								curr_time.tv_sec - last_time.tv_sec - 1,curr_time.tv_usec - last_time.tv_usec + 1000000);
		}
	}
	free(elem_arry);
	printf("silc_mcache_add_new_mblk free_count = %u\n",blk->free_count);
	free(free_array);
	free(bit_flag);
}

void silc_test_memory_access(test_lib_arg* arg,int* code_array,int array_length)
{
	silc_bool is_set_value = silc_false;
	int i = 0;
	char* elem_value = NULL;
	uint32_t elem_size = 0;
	uint32_t create_size = 0;
	uint32_t mem_align = 0;
	silc_mcache* mem_cache = NULL;
	while(code_array[i] >= 0)
	{
		switch (code_array[i])
		{
		    case TEST_MEM_ALIGN:
		    	mem_align = arg->mem_align;
		    	printf("memory alignment mem_align= %u\n",mem_align);
		    case TEST_ELEM_VALUE:
		    	elem_value = arg->elem_value;
		    	if(elem_value == NULL || 0 == strlen(elem_value))
		    	{
		    		break;
		    	}
		    	printf("elem_value = %s\n",elem_value);
		    	is_set_value = silc_true;
		    	break;
		    case TEST_ELEM_SIZE:
		    	elem_size = arg->elem_size;
		    	printf("TEST_ELEM_SIZE elem_size = %u\n",elem_size);
		   		break;
		    case TEST_CREATE_SIZE:
		    	if(elem_size == 0)
		    	{
		    		if(!is_set_value)
		    		{
		    			elem_size = strlen(elem_value) + 1;
		    		}
		    		else
		    		{
			    		elem_value = "123456789";
			    		elem_size = strlen(elem_value) + 1;
		    		}
		    	}
		    	printf("elem_size = %u\n",elem_size);
		    	create_size = arg->create_size;
		    	mem_cache = silc_mcache_create("test_cache",elem_size,mem_align,create_size);
		    	if(mem_cache == NULL)
		    	{
		    		printf("memory cache create failed\n");
		    		return;
		    	}
		    	printf("memory cache create success,name = %s,elem_size = %u,grow = %u, mblk_size = %u, align = %u, mblk_cnt = %u\n",
		    			mem_cache->name,mem_cache->elem_size,mem_cache->grow,mem_cache->mblk_size,mem_cache->align,mem_cache->mblk_cnt);
				break;
		    case TEST_MEM_CAP:
		    	 printf("TEST_MEM_CAP test start\n");
		    	 if(mem_cache == NULL)
		    	 {
		    		 printf("TEST_MEM_CAP memory cache must create\n");
		    		 return;
		    	 }
		    	 silc_test_memory_cap(mem_cache,arg->mem_times);
		    	 printf("TEST_MEM_CAP test end\n");
		    	 return;
		   		 break;
		    default:
		    	printf("err argv\n");
		    	return;
		}
		i++;
	}
	uint32_t i_value =  1;
	int  v_size =(int)elem_size;
    while(v_size -2 > 0)
    {
    	i_value *= 10;
    	v_size--;
    }
    printf("i_value = %u\n",i_value);
    uint32_t j = 0;
    silc_mcache_blk* blk = silc_list_entry(mem_cache->mblk_free_list.next, silc_mcache_blk, usage_node);
    while(j < create_size)
    {
    	char* p_char = (char*)malloc(elem_size);
    	memset(p_char,0,elem_size);
    	sprintf(p_char,"%u",i_value + j);
    	*(p_char + elem_size - 1) = '\0';
    	//printf("%s\n",p_char);
    	char* m_char = silc_mcache_alloc_real(mem_cache);
    	memcpy(m_char,p_char,elem_size);
    	if(0 != silc_mcache_free_real(m_char))
    	{
    		free(p_char);
    		return;
    	}
        free(p_char);
    	j++;
    }

    printf("free_count = %u\n", blk->free_count);
    silc_slist_node* cur = blk->free_list.first;
    uint32_t count = blk->free_count;
    while(count != 0)
    {
    //	silc_mcache_node* ret_node = silc_slist_entry(cur, silc_mcache_node, snode);
    //	char* ret;
    //	ret = (char*)(ret_node + 1);
    	cur = cur->next;
    	count--;
    //	printf("%s ",ret);
    }
    printf("\n");
    printf("start \n");
  //  sleep(2);
    uint32_t times = 0;//6000000;
    while(times)
    {
    	silc_mcache_add_new_mblk(mem_cache);
    	times--;
    }
    printf("mem_cache->mblk_cnt = %u \n",mem_cache->mblk_cnt);
    silc_mcache_destroy(mem_cache);
    printf("end \n");
 //   sleep(5);
   /* if(is_set_value)
    {
    	printf("free(elem_value) \n");
    	free(elem_value);
    }*/
    return;
	//for(i = 0 ; i < 100)
}
