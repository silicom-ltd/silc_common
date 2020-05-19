/*
 * silc_stats.c
 *
 *  Created on: Jul 14, 2012
 *      Author: jeff_zheng
 */

#include "silc_common.h"



#define silc_stats_acc_all_type_def(data_type, name_type)\
int silc_stats_acc_dis_init_##name_type(silc_stats_acc* p_acc)\
{\
	uint32_t field_curr;\
	uint32_t instance_curr;\
	silc_bool err_flag = silc_false;\
	for(instance_curr = 0;instance_curr < p_acc->instance_cnt; instance_curr++)\
	{\
		for(field_curr = 0;field_curr < p_acc->field_cnt;field_curr++)\
		{\
			if(p_acc->acc_types[field_curr] == SILC_STATS_ACC_DIS)\
			{\
				silc_stats_acc_distribute* persent_dis = p_acc->acc_dis + instance_curr * p_acc->field_cnt + field_curr;\
				persent_dis->dis_htbl = silc_htbl_create(SILC_STAT_DIS_HTBL_BITS,silc_stats_acc_dis_hash_##name_type, silc_stats_acc_dis_cmp_key_##name_type,2048);\
				SILC_LOG("silc_htbl_create");\
				if(persent_dis->dis_htbl == NULL)\
				{\
					err_flag = silc_true;\
					SILC_ERR("silc_stats_acc_dis_init()  silc_htbl_create failed");\
					break;\
				}\
				silc_slist_init(&persent_dis->dis_list);\
			}\
		}\
		if(err_flag)\
			break;\
	}\
	if(err_flag)\
	{\
		silc_stats_acc_dis_array_clear(p_acc,instance_curr + 1,field_curr);\
		SILC_ERR("silc_stats_acc_dis_init()  initial (silc_stats_acc_distribute*) acc_dis failed");\
		return -1;\
	}\
	return 0;\
}\
void silc_stats_acc_dis_list_sort_##name_type(silc_stats_acc_distribute* p_dis)\
{\
	silc_slist_qsort(&p_dis->dis_list,silc_stats_acc_node_cmp_##name_type);\
}


#define silc_stats_acc_type_def(data_type, name_type) \
data_type silc_stats_acc_get_val_##name_type (silc_stats_acc* p_acc, uint32_t inst, uint32_t field) \
{ \
	void* p_data = (p_acc->acc_data+(inst*p_acc->field_cnt + field))->acc_data; \
		return *(data_type*)p_data;\
} \
int silc_stats_acc_dis_insert_elem_##name_type(silc_stats_acc_distribute* p_dis,data_type new_value) \
{ \
	if(p_dis->dis_htbl == NULL)\
		 return -1;\
	silc_htbl_node* p_node = silc_htbl_get_node(p_dis->dis_htbl,&new_value); \
	if(p_node == NULL)  \
	{ \
		silc_stats_acc_dis_elem* p_elem = (silc_stats_acc_dis_elem*)calloc(1,sizeof(silc_stats_acc_dis_elem)); \
		p_elem->value = (data_type*)malloc(sizeof(data_type)); \
		*((data_type*)p_elem->value) = new_value; \
		p_elem->count = 1; \
		if(0 != silc_htbl_insert(p_dis->dis_htbl, p_elem->value,p_elem)) \
		{ \
			free(p_elem->value); \
			free(p_elem); \
			return -1; \
		} \
		silc_slist_add_head(&p_elem->node,&p_dis->dis_list); \
	} \
	else \
	{ \
		((silc_stats_acc_dis_elem*)(p_node->p_elem))->count++; \
	} \
	return 0; \
} \
void silc_stats_acc_cal_acc_## name_type ( silc_stats_acc* p_acc, uint32_t inst, uint32_t field, data_type new_value) \
{\
	uint32_t idx = (inst*p_acc->field_cnt) + field; \
	silc_stats_acc_type acc_type = p_acc->acc_types[field]; \
	silc_stats_acc_unit* p_unit = p_acc->acc_data + idx;\
	silc_stats_acc_distribute* p_dis = p_acc->acc_dis + idx; \
	data_type* p_raw = (data_type*)(p_unit->raw_data); \
	data_type* p_ret = (data_type*)(p_unit->acc_data); \
	p_unit->count++; \
	if(p_unit->count==1) { \
		*p_raw = new_value; \
		*p_ret = (*p_raw); \
		if(acc_type == SILC_STATS_ACC_DIS)\
             silc_stats_acc_dis_insert_elem_ ## name_type(p_dis,new_value); \
	} \
	else \
	{ \
		switch(acc_type) \
		{\
		case 	SILC_STATS_ACC_UNK:  \
			printf("unknow acc type"); \
			break; \
		case 	SILC_STATS_ACC_MAX:  \
			if(*p_raw < new_value)	{ \
				*p_raw = new_value; \
			} \
			*p_ret = (*p_raw); \
			break; \
		case 	SILC_STATS_ACC_MIN: \
			if(*p_raw > new_value)	{ \
				*p_raw = new_value; \
			} \
			*p_ret = (*p_raw); \
			break; \
		case 	SILC_STATS_ACC_MEAN: \
			*p_raw += new_value; \
			*p_ret = (*p_raw) / p_unit->count; \
			break; \
		case 	SILC_STATS_ACC_SUM: \
			*p_raw += new_value; \
			*p_ret = (*p_raw); \
			break; \
		case	SILC_STATS_ACC_LAST:\
		     *p_raw = new_value; \
		     *p_ret = (*p_raw); \
		     break;\
		case	SILC_STATS_ACC_DIS:\
		     silc_stats_acc_dis_insert_elem_ ## name_type(p_dis,new_value); \
		     break;\
		} \
	}\
\
}\
void silc_stats_acc_push_## name_type(silc_stats_acc* p_acc, uint32_t inst, uint32_t field, data_type value) \
{ \
	uint32_t idx = (inst*p_acc->field_cnt) + field; \
	data_type v = value;\
	if(p_acc->type==SILC_STATS_ACC_VAL_INTERVAL)\
	{ /*interval, we should calculate the interval value here*/\
		if(p_acc->interval_init[idx]==0) \
		{/*one time init for a single field*/\
			p_acc->interval_init[idx] = 1;\
			/*initialize old_value, and not calling the cal_acc functions*/ \
			*SILC_PTR_OFFSET(p_acc->old_value, idx*sizeof(uint64_t), data_type) = value; \
			return; \
		}\
		v = value - (*SILC_PTR_OFFSET(p_acc->old_value, idx*sizeof(uint64_t), data_type)); \
	} \
	*SILC_PTR_OFFSET(p_acc->old_value, idx*sizeof(uint64_t), data_type) = value; \
	silc_stats_acc_cal_acc_## name_type(p_acc, inst, field, v); \
\
	return; \
}\
void silc_stats_acc_print_result_## name_type(silc_stats_acc* p_acc,char* field_name,uint16_t prnt_width) \
{ \
	uint32_t instance_cnt; \
	char pnt_str[16] = {0}; \
	SILC_STAT_FIELD_PRINT_FORMAT_WIDTH(pnt_str,prnt_width,string); \
	char pnt_uint64[16] = {0}; \
	SILC_STAT_FIELD_PRINT_FORMAT_WIDTH(pnt_uint64,prnt_width,uint64); \
	char pnt_name_type[16] = {0}; \
	SILC_STAT_FIELD_PRINT_FORMAT_WIDTH(pnt_name_type,prnt_width,name_type); \
\
	uint16_t bar_cnt = prnt_width * (p_acc->field_cnt + 3);\
	char* bar_str = (char *) malloc(bar_cnt + 1);\
	memset(bar_str,'-',bar_cnt);\
	bar_str[bar_cnt] = 0;\
\
	for(instance_cnt = 0;instance_cnt < p_acc->instance_cnt;instance_cnt++) \
	{ \
		printf("%s\n",bar_str);  \
		printf(pnt_str,"field name"); \
		printf(pnt_str,"count"); \
		uint32_t inst_end = instance_cnt * p_acc->field_cnt;  \
		uint32_t field_cnt; \
		for(field_cnt = 0;field_cnt < p_acc->field_cnt;field_cnt++) \
		{ \
			switch(p_acc->acc_types[field_cnt]) \
			{ \
				case SILC_STATS_ACC_UNK: \
					printf(pnt_str,"unknow"); \
					break;\
				case SILC_STATS_ACC_MAX: \
				    printf(pnt_str,"max");\
				    break;\
				case SILC_STATS_ACC_MIN:\
					printf(pnt_str,"min");\
					break;\
				case SILC_STATS_ACC_MEAN:\
					printf(pnt_str,"average");\
					break;\
				case SILC_STATS_ACC_SUM:\
					printf(pnt_str,"total");\
					break;\
				case SILC_STATS_ACC_LAST:\
					printf(pnt_str,"last");\
					break;\
				case SILC_STATS_ACC_DIS:\
				     break;\
			}\
		}\
		printf("|\n");\
		printf(pnt_str,field_name);\
		printf(pnt_uint64,p_acc->acc_data[inst_end * p_acc->field_cnt].count);\
		for(field_cnt = 0;field_cnt < p_acc->field_cnt;field_cnt++)\
		{\
			data_type value = silc_stats_acc_get_val_## name_type(p_acc,instance_cnt,field_cnt);\
			switch(p_acc->acc_types[inst_end + field_cnt])\
			{\
			    case SILC_STATS_ACC_UNK:\
					printf(pnt_str,"0");\
					break;\
				case SILC_STATS_ACC_MAX:\
			    case SILC_STATS_ACC_MIN:\
				case SILC_STATS_ACC_MEAN:\
				case SILC_STATS_ACC_SUM:\
				case SILC_STATS_ACC_LAST:\
					printf(pnt_name_type,value);\
					break;\
				case SILC_STATS_ACC_DIS:\
				     break;\
			}\
		}\
		printf("|\n");\
		printf("%s\n\n",bar_str);\
	}\
	free(bar_str);\
}\
uint32_t silc_stats_acc_dis_hash_##name_type(const void* p_key)\
{\
	data_type tmp_key = *(data_type*)p_key;\
	uint32_t value = (uint64_t)(tmp_key * 11400714819323198485ULL) >> (64 - SILC_STAT_DIS_HTBL_BITS);\
	return value;\
}\
int silc_stats_acc_dis_cmp_key_##name_type(const void* p_key1, const void* p_key2)\
{\
	if(*(data_type*)p_key1 != *(data_type*)p_key2)\
	{\
		return -1;\
	}\
	else\
	{\
		return 0;\
	}\
}\
int silc_stats_acc_node_cmp_##name_type(silc_slist_node* p_node1, silc_slist_node* p_node2)\
{\
	silc_stats_acc_dis_elem* elem1= silc_slist_entry(p_node1,silc_stats_acc_dis_elem,node);\
	silc_stats_acc_dis_elem* elem2= silc_slist_entry(p_node2,silc_stats_acc_dis_elem,node);\
\
	if(*(data_type*)elem1->value > *(data_type*)elem2->value)\
	{\
		return -1;\
	}\
	else if(*(data_type*)elem1->value < *(data_type*)elem2->value)\
	{\
		return 1;\
	}\
	else\
	{\
		return 0;\
	}\
}\
void  silc_stats_acc_print_dis_comma_##name_type(silc_stats_acc_distribute* p_dis)\
{ \
	silc_slist_node* p_node = p_dis->dis_list.first;\
	while(p_node != NULL)\
	{\
		silc_stats_acc_dis_elem* p_elem = silc_slist_entry(p_node,silc_stats_acc_dis_elem,node);\
		if(p_elem == NULL)\
		{\
			SILC_ERR("data structure create failed");\
			return ;\
		}\
		printf(SILC_STAT_FIELD_PRINT_FORMAT(name_type)",%"SILC_STAT_FIELD_PRINT_FORMAT_uint64";",*((data_type*)p_elem->value),p_elem->count);\
		p_node = p_node->next;\
	}\
	printf("\n");\
}\
void silc_stats_acc_print_dis_bar_##name_type(silc_stats_acc_distribute* p_dis,uint16_t prnt_width,uint16_t line_count)\
{\
	char fm_name[16] = {0};\
	char fm_uint32[16] = {0};\
	char fm_uint64[16] = {0};\
	SILC_STAT_FIELD_PRINT_FORMAT_WIDTH(fm_name,prnt_width,name_type);\
	SILC_STAT_FIELD_PRINT_FORMAT_WIDTH(fm_uint64,prnt_width,uint64);\
	SILC_STAT_FIELD_PRINT_FORMAT_WIDTH(fm_uint32,prnt_width,uint32);\
\
	uint16_t bar_cnt = prnt_width * line_count;\
	char* bar_str = (char *) malloc(bar_cnt + 1);\
	memset(bar_str,'-',bar_cnt);\
	bar_str[bar_cnt] = 0;\
 \
	uint16_t i = 0;\
	silc_slist_node* p_node = p_dis->dis_list.first;\
 \
	uint64_t* cnt_arry = (uint64_t*)malloc(line_count * sizeof(uint64_t));\
	while(p_node != NULL)\
	{\
		silc_stats_acc_dis_elem* p_elem = silc_slist_entry(p_node,silc_stats_acc_dis_elem,node);\
		if(p_elem == NULL)\
		{\
			SILC_ERR("data structure create failed");\
			return ;\
		}\
		if(0 == i)\
		{\
			printf("%s\n",bar_str);\
		}\
		*(cnt_arry + i) = p_elem->count;\
		printf(fm_name,*((data_type*)p_elem->value));\
		i++;\
		if(i == line_count)\
		{\
			printf("\n");\
			uint16_t j = 0;\
			while(j < i)\
			{\
				printf(fm_uint64,cnt_arry[j]);\
				j++;\
			}\
			printf("\n");\
			i = 0;\
		}\
		p_node = p_node->next;\
	}\
	printf("\n");  \
	uint16_t j = 0;\
	if(i != 0) \
	{\
		while(j < i)\
		{\
			printf(fm_name,cnt_arry[j]);\
			j++;\
		}\
		printf("\n");\
	}\
	printf("%s\n",bar_str);\
	printf("\n");\
	free(bar_str);\
	free(cnt_arry);\
}\
void silc_stats_acc_print_dis_array_##name_type(silc_stats_acc* p_acc,char* field_name,silc_bool flag,uint16_t prnt_width,uint16_t line_count)\
{\
	uint32_t instance_cnt;\
	uint32_t field_cnt;\
	for(instance_cnt = 0;instance_cnt < p_acc->instance_cnt;instance_cnt++)\
	{\
		uint32_t inst_end = instance_cnt * p_acc->field_cnt;\
		for(field_cnt = 0;field_cnt < p_acc->field_cnt;field_cnt++)\
		{\
			if(p_acc->acc_types[field_cnt] == SILC_STATS_ACC_DIS)\
			{\
				silc_stats_acc_distribute* p_dis = p_acc->acc_dis + inst_end + field_cnt;\
				silc_stats_acc_unit* p_unit = p_acc->acc_data + inst_end + field_cnt;\
				silc_stats_acc_dis_list_sort_##name_type(p_dis);\
				printf("field name: %s     count: %"SILC_STAT_FIELD_PRINT_FORMAT_uint64"\n",field_name,p_unit->count);\
				if(flag)\
				{\
					silc_stats_acc_print_dis_bar_##name_type(p_dis,prnt_width,line_count);\
				}\
				else\
				{\
					silc_stats_acc_print_dis_comma_##name_type(p_dis);\
				}\
			}\
		}\
	}\
}

static inline int silc_stats_acc_dis_insert_elem_string(silc_stats_acc_distribute* p_dis,char* new_value)
{
	if(p_dis->dis_htbl == NULL)
		 return -1;
	silc_htbl_node* p_node = silc_htbl_get_node(p_dis->dis_htbl,new_value);
	if(p_node == NULL)
	{
		silc_stats_acc_dis_elem* p_elem = (silc_stats_acc_dis_elem*)calloc(1,sizeof(silc_stats_acc_dis_elem));
		uint16_t len =strlen(new_value);
		p_elem->value = (void *)malloc(len + 1);
		memcpy(p_elem->value,new_value,len);
		*((char*)p_elem->value + len) = '\0';
		p_elem->count = 1;
		if(0 != silc_htbl_insert(p_dis->dis_htbl, p_elem->value,p_elem))
		{
			free(p_elem->value);
			free(p_elem);
			return -1;
		}
		silc_slist_add_head(&p_elem->node,&p_dis->dis_list);
	}
	else
	{
		((silc_stats_acc_dis_elem*)(p_node->p_elem))->count++;
	}
	return 0;
}

static inline void  silc_stats_acc_print_dis_comma_string(silc_stats_acc_distribute* p_dis)
{
	silc_slist_node* p_node = p_dis->dis_list.first;
	while(p_node != NULL)
	{
		silc_stats_acc_dis_elem* p_elem = silc_slist_entry(p_node,silc_stats_acc_dis_elem,node);
		if(p_elem == NULL)
		{
			SILC_ERR("data structure create failed");
			return ;
		}
		printf("%s,%"SILC_STAT_FIELD_PRINT_FORMAT_uint64";",(char*)p_elem->value,p_elem->count);
		p_node = p_node->next;
	}
	printf("\n");
}

static inline void silc_stats_acc_print_dis_bar_string(silc_stats_acc_distribute* p_dis,uint16_t prnt_width,uint16_t line_count)
{
	char fm_name[16] = {0};
	char fm_uint32[16] = {0};
	char fm_uint64[16] = {0};
	SILC_STAT_FIELD_PRINT_FORMAT_WIDTH(fm_name,prnt_width,string);
	SILC_STAT_FIELD_PRINT_FORMAT_WIDTH(fm_uint64,prnt_width,uint64);
	SILC_STAT_FIELD_PRINT_FORMAT_WIDTH(fm_uint32,prnt_width,uint32);

	uint16_t bar_cnt = prnt_width * line_count;
	char* bar_str = (char *) malloc(bar_cnt + 1);
	memset(bar_str,'-',bar_cnt);
	bar_str[bar_cnt] = 0;

	uint16_t i = 0;
	silc_slist_node* p_node = p_dis->dis_list.first;

	uint64_t* cnt_arry = (uint64_t*)malloc(line_count * sizeof(uint64_t));
	while(p_node != NULL)
	{
		silc_stats_acc_dis_elem* p_elem = silc_slist_entry(p_node,silc_stats_acc_dis_elem,node);\
		if(p_elem == NULL)
		{
			SILC_ERR("data structure create failed");
			return ;
		}
		if(0 == i)
		{
			printf("%s\n",bar_str);
		}
		*(cnt_arry + i) = p_elem->count;
		printf(fm_name,(char*)p_elem->value);

		i++;

		if(i == line_count)
		{
			printf("\n");
			uint16_t j = 0;
			while(j < i)
			{
				printf(fm_uint64,cnt_arry[j]);
				j++;
			}
			printf("\n");
			i = 0;
		}
		p_node = p_node->next;
	}
	printf("\n");
	uint16_t j = 0;
	if(i != 0)
	{
		while(j < i)
		{
			printf(fm_uint64,cnt_arry[j]);
			j++;
		}
    	printf("\n");
	}
	printf("%s\n",bar_str);
	printf("\n");
	free(bar_str);
	free(cnt_arry);
}

static inline int silc_stats_acc_node_cmp_string(silc_slist_node* p_node1, silc_slist_node* p_node2)
{
	silc_stats_acc_dis_elem* elem1 = silc_slist_entry(p_node1,silc_stats_acc_dis_elem,node);
	silc_stats_acc_dis_elem* elem2 = silc_slist_entry(p_node2,silc_stats_acc_dis_elem,node);

	if(elem1->count > elem2->count)
	{
		return -1;
	}
	else if(elem1->count < elem2->count)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}





silc_stats_acc_all_type_def(float, float);
silc_stats_acc_all_type_def(double, double);
silc_stats_acc_all_type_def(uint32_t, uint32);
silc_stats_acc_all_type_def(int32_t, int32);
silc_stats_acc_all_type_def(int64_t, int64);
silc_stats_acc_all_type_def(uint64_t, uint64);
silc_stats_acc_all_type_def(char*, string);


static inline void silc_stats_acc_print_dis_array_string(silc_stats_acc* p_acc,char* field_name,silc_bool flag,uint16_t prnt_width,uint16_t line_count)
{
	uint32_t instance_cnt;
	uint32_t field_cnt;
	for(instance_cnt = 0;instance_cnt < p_acc->instance_cnt;instance_cnt++)
	{
		uint32_t inst_end = instance_cnt * p_acc->field_cnt;
		for(field_cnt = 0;field_cnt < p_acc->field_cnt;field_cnt++)
		{
			if(p_acc->acc_types[field_cnt] == SILC_STATS_ACC_DIS)
			{
				silc_stats_acc_distribute* p_dis = p_acc->acc_dis + inst_end + field_cnt;
				silc_stats_acc_unit* p_unit = p_acc->acc_data + inst_end + field_cnt;
				silc_stats_acc_dis_list_sort_string(p_dis);
				printf("field name: %s     count: %"SILC_STAT_FIELD_PRINT_FORMAT_uint64"\n",field_name,p_unit->count);
				if(flag)
				{
					silc_stats_acc_print_dis_bar_string(p_dis,prnt_width,line_count);
				}
				else
				{
					silc_stats_acc_print_dis_comma_string(p_dis);
				}
			}
		}
	}
}


silc_stats_acc_type_def(float, float);
silc_stats_acc_type_def(double, double);
silc_stats_acc_type_def(uint32_t, uint32);
silc_stats_acc_type_def(int32_t, int32);
silc_stats_acc_type_def(int64_t, int64);
silc_stats_acc_type_def(uint64_t, uint64);

static inline void silc_stats_acc_push_string(silc_stats_acc* p_acc, uint32_t inst, uint32_t field, char* value)
{
	uint32_t idx = (inst*p_acc->field_cnt) + field;
	silc_stats_acc_distribute* p_dis = p_acc->acc_dis + idx;
	silc_stats_acc_unit* p_unit = p_acc->acc_data + idx;
	p_unit->count++;
	silc_stats_acc_dis_insert_elem_string(p_dis,value);
	return;
}
