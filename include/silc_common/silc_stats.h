/*
 * silc_stats.h
 *
 *  Created on: Mar 24, 2011
 *      Author: jeff_zheng
 *   Copyright: NetPerform Technology
 */

#ifndef SILC_STATS_H_
#define SILC_STATS_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>

#define silc_stats_acc_type_def_extern(data_type, name_type)\
uint32_t silc_stats_acc_dis_hash_##name_type(const void* p_key);\
int silc_stats_acc_dis_cmp_key_##name_type(const void* p_key1, const void* p_key2);\
int silc_stats_acc_node_cmp_##name_type(silc_slist_node* p_node1, silc_slist_node* p_node2);

#define SILC_STAT_FIELD_PRINT_FORMAT_uint64 PRIu64
#define SILC_STAT_FIELD_PRINT_FORMAT_uint32 PRIu32

#define SILC_STAT_FIELD_PRINT_FORMAT_int32  PRId32

#define SILC_STAT_FIELD_PRINT_FORMAT_int64  PRId64

#define SILC_STAT_FIELD_PRINT_FORMAT_string  "s"

#define SILC_STAT_FIELD_PRINT_FORMAT_float  "f"

#define SILC_STAT_FIELD_PRINT_FORMAT_double  "e"

#define SILC_STAT_FIELD_PRINT_FORMAT_WIDTH(str,width,nametype) snprintf(str,sizeof(str),"|%%-%u"SILC_STAT_FIELD_PRINT_FORMAT_##nametype,width)

#define SILC_STAT_FIELD_PRINT_FORMAT(nametype) "%"SILC_STAT_FIELD_PRINT_FORMAT_##nametype
#define SILC_STAT_DIS_HTBL_BITS 10
/**
 * @defgroup silc_stats_acc silc_stats_acc
 * @brief a quick stats accumulator
 * @ingroup silc_common
 *
 * used to monitor a incremental value and create interval values from historical values
 * the accumulator can monitor a set of instance, and each instance can have multiple fields
 */

/*
 *
 */
typedef enum silc_stats_acc_val_e
{
	SILC_STATS_ACC_VAL_INTERVAL = 0,
	SILC_STATS_ACC_VAL_CURRENT  = 1
}silc_stats_acc_value;

typedef enum silc_stats_acc_type_e
{
	SILC_STATS_ACC_UNK = 0,
	SILC_STATS_ACC_MAX = 1,
	SILC_STATS_ACC_MIN = 2,
	SILC_STATS_ACC_MEAN = 3,
	SILC_STATS_ACC_SUM = 4,
	SILC_STATS_ACC_LAST = 5,
	SILC_STATS_ACC_DIS = 6
}silc_stats_acc_type;

typedef struct silc_stats_acc_unit_s
{
	uint8_t acc_data[8];
	uint8_t raw_data[8];
	uint64_t count;
}silc_stats_acc_unit;

typedef struct silc_stats_acc_distribute_s
{
	silc_htbl*  dis_htbl;
	silc_slist  dis_list;
}silc_stats_acc_distribute;

typedef struct silc_stats_acc_s
{
	silc_stats_acc_value 	type;
	uint32_t 			instance_cnt;
	uint32_t 			field_cnt;
	silc_stats_acc_type*	acc_types;
	silc_stats_acc_distribute* acc_dis;
	silc_stats_acc_unit*	acc_data;
	void* 				old_value;
	char* 				interval_init;
}silc_stats_acc;

typedef struct silc_stats_acc_dis_elem_s
{
	void* value;
    uint64_t count;
    silc_slist_node node;
}silc_stats_acc_dis_elem;

silc_stats_acc_type_def_extern(float, float)
silc_stats_acc_type_def_extern(double, double)
silc_stats_acc_type_def_extern(uint32_t, uint32)
silc_stats_acc_type_def_extern(int32_t, int32)
silc_stats_acc_type_def_extern(int64_t, int64)
silc_stats_acc_type_def_extern(uint64_t, uint64)

static inline uint32_t silc_stats_acc_dis_hash_string(const void* key)
{
	const char* str = (char*)key;
	uint32_t seed = 131; /* 31 131 1313 13131 131313 etc.. */
	uint32_t hash = 0;
	uint32_t i = 0;
	while(*(str + i) != '\0')
	{
		hash = (hash * seed) + *(str + i);
		i++;
	}
	hash >>= (32 - SILC_STAT_DIS_HTBL_BITS);
	return hash;
}

static inline int silc_stats_acc_dis_cmp_key_string(const void* p_key1, const void* p_key2)
{
	return strcmp((char*)p_key1,(char*)p_key2);
}

static inline void silc_stats_acc_dis_clear(silc_stats_acc_distribute* dis)
{
	if(dis->dis_htbl != NULL)
	{
		silc_htbl_destroy(dis->dis_htbl);
		silc_slist_node* p_del = NULL;
	    while(NULL != (p_del = silc_slist_del_head(&dis->dis_list)))
	    {
		    silc_stats_acc_dis_elem* del_elem = silc_slist_entry(p_del,silc_stats_acc_dis_elem,node);
		    free(del_elem->value);
		    free(del_elem);
	    }
	}
}

static inline void silc_stats_acc_dis_array_clear(silc_stats_acc* p_acc,uint32_t instance_max,uint32_t field_max)
{
	uint32_t field_tmp;
	uint32_t instance_tmp;
	for(instance_tmp = 0;instance_tmp < instance_max; instance_tmp++)
	{
		uint32_t instance_end = instance_tmp * p_acc->field_cnt;
		for(field_tmp = 0;field_tmp < field_max;field_tmp++)
		{
			if(p_acc->acc_types[field_tmp] == SILC_STATS_ACC_DIS)
			{
				silc_stats_acc_distribute* persent_dis = p_acc->acc_dis + instance_end + field_tmp;
				silc_stats_acc_dis_clear(persent_dis);
			}
		}
	}
}

/**
 * @ingroup silc_stats_acc
 * @brief Uninitialize a stats accumulator
 *
 * @param[in] p_acc 	        struct to initialize
 * @return 0 for success, non-zero for Error
 *
 * Uninitialize a stats accumulator, free all resources.
 **/
static inline void silc_stats_acc_destroy(silc_stats_acc* p_acc)
{
	if(p_acc)
	{
		silc_stats_acc_dis_array_clear(p_acc,p_acc->instance_cnt,p_acc->field_cnt);
		free(p_acc);
	}
}
/**
 * @ingroup silc_stats_acc
 * @brief initialize a stats accumulator
 *
 * @param[in] p_acc 	        struct to initialize
 * @param[in] instance_count 	number of instances
 * @param[in] field_count 	    number of fields per instance
 * @return the created stats_acc obj, NULL for Error
 *
 **/
static inline silc_stats_acc* silc_stats_acc_create(silc_stats_acc_value type, uint32_t instance_count,
		         silc_stats_acc_type* field_acc_types, uint32_t field_count)
{
	uint32_t total_size = sizeof(silc_stats_acc) +
						sizeof(silc_stats_acc_type)*field_count +
						sizeof(uint64_t)*(instance_count*field_count*2) +
						sizeof(silc_stats_acc_distribute)*field_count*instance_count+
						sizeof(silc_stats_acc_unit)*(instance_count*field_count);
	silc_stats_acc* p_acc = (silc_stats_acc*)malloc(total_size);
	if(p_acc==NULL)
		return NULL;
	memset(p_acc, 0, total_size);
	p_acc->type = type;
	p_acc->instance_cnt = instance_count;
	p_acc->field_cnt = field_count;
	p_acc->old_value = SILC_PTR_OFFSET(p_acc, sizeof(*p_acc), void);
	p_acc->acc_types = SILC_PTR_OFFSET(p_acc->old_value,  (instance_count*field_count*sizeof(uint64_t)), silc_stats_acc_type);
	memcpy(p_acc->acc_types, field_acc_types, sizeof(silc_stats_acc_type)* field_count);

	p_acc->acc_dis = SILC_PTR_OFFSET(p_acc->acc_types, sizeof(silc_stats_acc_type)*(field_count), silc_stats_acc_distribute);

	p_acc->acc_data = SILC_PTR_OFFSET(p_acc->acc_dis, sizeof(silc_stats_acc_distribute)*field_count*instance_count, silc_stats_acc_unit);
	p_acc->interval_init = SILC_PTR_OFFSET(p_acc->acc_data, (instance_count*field_count*sizeof(silc_stats_acc_unit)), char);

	return p_acc;
}


/**
 * @ingroup silc_stats_acc
 * @brief set initial value for off field
 *
 * @param[in] p_acc 	        acc struct
 * @param[in] inst 				instance index
 * @param[in] field 	    	field index
 * @param[in] value 	    	initial value
 * @return 0 for success, non-zero for Error
 *
 * after initialization all fields are set to 0, but the user data may already have an existing value.
 * In such case, the caller should call this function once per instance per field with the existing value.
 **/
//static inline int silc_stats_acc_set_initial_val(silc_stats_acc* p_acc, uint32_t inst, uint32_t field, uint64_t value)
//{
////	*(p_acc->old_value+(inst*p_acc->field_cnt + field)) = value;
//	return 0;
//}

/**
 * @ingroup silc_stats_acc
 * @brief set initial value for off field
 *
 * @param[in] p_acc 	        acc struct
 * @param[in] inst 				instance index
 * @param[in] field 	    	field index
 * @param[in] value 	    	new value to be pushed
 * @return 0 for success, non-zero for Error
 *
 * the caller should call this function periodically with a new value each time.
 * the accumulator will then use the new value to generate the value the caller needed.
 * In such case, the caller should call this function once per instance per field with the existing value.
 **/
//static inline uint64_t silc_stats_acc_push_new(silc_stats_acc* p_acc, uint32_t inst, uint32_t field, uint64_t value)
//{
//	uint32_t idx = (inst*p_acc->field_cnt) + field;
//	uint64_t v = value - (*(p_acc->old_value+idx));
//	*(p_acc->interval_value+idx) = v;
//	*(p_acc->old_value+idx) = value;
//	return 0;
//}

/**
 * @ingroup silc_stats_acc
 * @brief get the last generated value
 *
 * @param[in] p_acc 	        acc struct
 * @param[in] inst 				instance index
 * @param[in] field 	    	field index
 * @return the last generated value, if no value has been generated, 0 is returned
 *
 * the caller should call this function periodically with a new value each time.
 * the accumulator will then use the new value to generate the value the caller needed.
 * In such case, the caller should call this function once per instance per field with the existing value.
 **/
//static inline uint64_t silc_stats_acc_get_val(silc_stats_acc* p_acc, uint32_t inst, uint32_t field)
//{
//	switch(p_acc->type)
//	{
//	case SILC_STATS_ACC_VAL_INTERVAL:
//		return *(p_acc->interval_value+(inst*p_acc->field_cnt + field));
//	case SILC_STATS_ACC_VAL_CURRENT:
//		return *(p_acc->old_value+(inst*p_acc->field_cnt + field));
//	}
//	return 0;
//}

#define silc_stats_acc_get_val_def(data_type) \


silc_stats_acc_get_val_def(float);
silc_stats_acc_get_val_def(double);
silc_stats_acc_get_val_def(uint32_t);
silc_stats_acc_get_val_def(uint64_t);



static inline void silc_stats_acc_clear(silc_stats_acc* p_acc)
{
	silc_stats_acc_dis_array_clear(p_acc,p_acc->instance_cnt,p_acc->field_cnt);
	memset(p_acc->acc_data, 0, sizeof(silc_stats_acc_unit)*p_acc->instance_cnt*p_acc->field_cnt);
}

static inline uint32_t silc_stats_acc_get_field_cnt(silc_stats_acc* p_acc)
{
	return p_acc->field_cnt;
}

static inline uint32_t silc_stats_acc_get_instance_cnt(silc_stats_acc* p_acc)
{
	return p_acc->instance_cnt;
}

#ifdef __cplusplus
}
#endif

#endif /* SILC_STATS_H_ */
