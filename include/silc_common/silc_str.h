/*
 * silc_str.h
 *
 *  Created on: Nov 23, 2010
 *      Author: jeff_zheng
 *   Copyright: NetPerform Technology
 */

#ifndef SILC_STR_H_
#define SILC_STR_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef char* silc_cstr;

typedef struct silc_str_s
{
	silc_cstr		str;
	uint32_t 	length;
}silc_str;

typedef struct silc_cstr_array_s
{
	silc_cstr* 	array;
	silc_bool* 	alloc_flags;

	uint32_t	max_size;
	uint32_t 	length;
	silc_bool		array_alloc_flag;
	silc_bool		flags_alloc_flag;
	silc_cstr		trailing_base;
	uint32_t	total_trailing_bytes;
	uint32_t	used_trailing_bytes;
}silc_cstr_array;


#define silc_str_declare(name, size) \
	char local_##name[size]; \
	silc_str name = {local_##name, size};


static inline silc_cstr silc_str_from_ip(silc_str* str_dst, size_t length, uint32_t addr)
{
	uint8_t* p_uint8 = (uint8_t*)&(addr);
	snprintf(str_dst->str, str_dst->length, "%u.%u.%u.%u",
			p_uint8[0],p_uint8[1],p_uint8[2],p_uint8[3]);
	str_dst->str[length-1]=0;
	return str_dst->str;
}

static inline char* silc_str_ip_to_str(char* dest, size_t length, uint32_t ipv4_addr)
{
	uint8_t* p_ip = (uint8_t*)&(ipv4_addr);
	snprintf(dest, length, "%u.%u.%u.%u", p_ip[0],p_ip[1],p_ip[2],p_ip[3]);
	dest[length-1]=0;
	return dest;
}

static inline  int silc_cstr_to_u32(const silc_cstr p_cstr,  uint32_t * ret_u32)
{
	char* endstr;
	uint32_t result;
	result = strtoul(p_cstr, &endstr, 10);
	if(endstr==p_cstr)
	{
		return -1;
	}
	if(*endstr!=0)
	{
		*ret_u32 = result;
		return -1;
	}
	*ret_u32 = result;
	return 0;
}

static inline  int silc_cstr_to_u32_partial(const silc_cstr p_cstr,  uint32_t * ret_u32)
{
	char* endstr;
	uint32_t result;
	result = strtoul(p_cstr, &endstr, 10);
	if(endstr==p_cstr)
	{
		return -1;
	}
	*ret_u32 = result;
	return 0;
}

static inline void silc_cstr_array_destroy(silc_cstr_array* p_array)
{
	free(p_array);
}
static inline silc_cstr_array* silc_cstr_array_create(uint32_t max_size, uint32_t max_bytes_total)
{
	silc_cstr_array* ret_array;
	uint32_t trailing_bytes = max_bytes_total*2;
	ret_array = (silc_cstr_array*)malloc(max_size*(sizeof(silc_bool)+sizeof(silc_cstr))+
										sizeof(silc_cstr_array)+trailing_bytes);

	if(ret_array==NULL)
		return NULL;
	ret_array->total_trailing_bytes = trailing_bytes;
	ret_array->array_alloc_flag = silc_true;
	ret_array->flags_alloc_flag = silc_true;
	ret_array->array = (silc_cstr*)(ret_array+1);
	ret_array->max_size = max_size;
	ret_array->alloc_flags = (silc_bool*)(ret_array->array+max_size);
	ret_array->length = 0;
	ret_array->used_trailing_bytes = 0;
	ret_array->trailing_base = (silc_cstr)(ret_array->alloc_flags + max_size);

	return ret_array;
}

static inline int silc_cstr_array_add(silc_cstr_array* p_array, const silc_cstr new_str)
{
	silc_cstr new_array_str;
	uint32_t new_str_len = strlen(new_str);
	if((p_array->used_trailing_bytes + new_str_len + 1)>p_array->total_trailing_bytes)
	{
		return -1;
	}
	if(p_array->length>=p_array->max_size)
	{
		return -1;
	}

	new_array_str = p_array->trailing_base + p_array->used_trailing_bytes;
	p_array->used_trailing_bytes += (new_str_len+1);
	p_array->array[p_array->length] = new_array_str;
	p_array->length ++;
	if(new_str_len)
		strncpy(new_array_str, new_str, new_str_len);
	new_array_str[new_str_len]=0;
	return 0;
}

static inline int silc_cstr_array_set_no_copy(silc_cstr_array* p_array, uint32_t index, const silc_cstr new_str)
{
	if(index >= p_array->length)
		return -1;

	p_array->array[index] = new_str;
	return 0;
}

static inline int silc_cstr_array_add_no_copy(silc_cstr_array* p_array, const silc_cstr new_str)
{
	if(p_array->length+1>p_array->max_size)
		return -1;

	p_array->array[p_array->length] = new_str;
	p_array->length ++;
	return 0;
}

static inline int silc_cstr_array_remove_last(silc_cstr_array* p_array)
{
	if(p_array->length == 0)
		return -1;

	p_array->array[p_array->length] = NULL;
	p_array->length ++;
	return 0;
}

static inline silc_cstr silc_cstr_strip_left(const silc_cstr src_cstr, char* tostrip)
{
	uint32_t src_len = strlen(src_cstr);
	uint32_t loop;
	uint32_t result_len;
	static char* default_strip = (char*)" \t";

	if(tostrip==NULL)
		tostrip = default_strip;
	uint32_t strip_len = strlen(tostrip);

	for(loop=0; loop<src_len; loop++)
	{
		uint32_t l = 0;
		silc_bool found = silc_false;
		for(l=0;l<strip_len;l++)
		{
			if(src_cstr[loop]==tostrip[l])
				found = silc_true;
		}
		if(!found)
			break;
	}
	result_len = src_len - loop;
	if(loop)
		memmove(src_cstr, src_cstr + loop, result_len);
	return src_cstr;
}
static inline silc_cstr silc_cstr_strip_right(silc_cstr src_cstr, char* tostrip)
{
	uint32_t src_len = strlen(src_cstr);
	uint32_t loop;
	static char* default_strip = (char*)" \t";

	if(tostrip==NULL)
		tostrip = default_strip;
	uint32_t strip_len = strlen(tostrip);

	for(loop=src_len; loop>0; loop--)
	{
		uint32_t l = 0;
		silc_bool found = silc_false;
		for(l=0;l<strip_len;l++)
		{
			if(src_cstr[loop-1]==tostrip[l])
			{
				src_cstr[loop-1] = 0;
				found = silc_true;
			}
		}
		if(!found)
			break;
	}
	return src_cstr;
}

static inline silc_cstr silc_cstr_strip_left_new(const silc_cstr src_cstr, char* tostrip)
{
	uint32_t src_len = strlen(src_cstr);
	uint32_t loop;
	uint32_t result_len;
	static char* default_strip = (char*)" \t";
	char* ret;

	if(tostrip==NULL)
		tostrip = default_strip;
	uint32_t strip_len = strlen(tostrip);

	for(loop=0; loop<src_len; loop++)
	{
		uint32_t l = 0;
		silc_bool found = silc_false;
		for(l=0;l<strip_len;l++)
		{
			if(src_cstr[loop]==tostrip[l])
				found = silc_true;
		}
		if(!found)
			break;
	}
	result_len = src_len - loop;
	ret = (char*)malloc(result_len + 8);
	if(ret)
	{
		if(result_len)
			strncpy(ret, src_cstr+loop, result_len);
		ret[result_len] = 0;
	}
	return ret;
}

static inline silc_cstr silc_cstr_strip_right_new(const silc_cstr src_cstr, char* tostrip)
{
	uint32_t src_len = strlen(src_cstr);
	uint32_t loop;
	static char* default_strip = (char*)" \t";
	char* ret;

	if(tostrip==NULL)
		tostrip = default_strip;
	uint32_t strip_len = strlen(tostrip);

	for(loop=src_len; loop>0; loop--)
	{
		uint32_t l = 0;
		silc_bool found = silc_false;
		for(l=0;l<strip_len;l++)
		{
			if(src_cstr[loop-1]==tostrip[l])
			{
				found = silc_true;
			}
		}
		if(!found)
		{
			break;
		}
	}
	ret = (char*)malloc(loop+8);
	if(ret)
	{
		if(loop)
			strncpy(ret, src_cstr, loop);
		ret[loop] = 0;
	}
	if(src_len-loop)
	{
		strcpy(ret, src_cstr+loop);
	}
	else
		ret[0] = 0;
	return ret;
}

static inline silc_cstr silc_cstr_strip_new(const silc_cstr src_cstr, char* tostrip)
{
	return silc_cstr_strip_left_new(src_cstr, tostrip);
}

static inline silc_cstr_array* silc_cstr_split(const silc_cstr src_str, char* splitter)
{
	uint32_t str_total_len = strlen(src_str);
	silc_cstr_array* ret_array;
	char* saveptr=NULL, *next_str;

	ret_array = silc_cstr_array_create(str_total_len/2+1, str_total_len+16);
	if(ret_array==NULL)
	{
		return NULL;
	}

	strcpy(ret_array->trailing_base, src_str);
	next_str = strtok_r(ret_array->trailing_base, splitter, &saveptr);
	while(next_str)
	{
		if(0!=silc_cstr_array_add_no_copy(ret_array, next_str))
		{
			silc_cstr_array_destroy(ret_array);
			return NULL;
		}
		next_str = strtok_r(NULL, splitter, &saveptr);
	}
	ret_array->trailing_base += str_total_len + 1;
	ret_array->used_trailing_bytes += str_total_len + 1;

	return ret_array;
}

static inline silc_cstr silc_cstr_array_get_quick(silc_cstr_array* arr, uint32_t arr_index)
{
	if(arr_index >=arr->length)
		return NULL;
	return arr->array[arr_index];
}

#define silc_cstr_array_for_each(substr, arr)	\
		do{uint32_t substrloop__; \
			for(substrloop__=0;substrloop__<arr->length;substrloop__++) {\
				substr = arr->array[substrloop__];
#define silc_cstr_array_for_each_end() }}while(0);

static inline char* silc_mprintf(const char *fmt, ...) __attribute__((format(printf, 1, 0)));
static inline char* silc_mprintf(const char *fmt, ...)
{
	char *ret = NULL;
	va_list ap;
	va_list ap_cp;
	int prn_size;
	char tmp_str[4];

	if(fmt==NULL)
		return strdup("");
	va_start(ap, fmt);
	va_copy(ap_cp, ap);
	prn_size = vsnprintf(tmp_str, 2, fmt, ap);
	if(prn_size >=0)
	{
		ret = malloc(prn_size + 8);
	}
	if(ret)
	{
		vsnprintf(ret, prn_size+1, fmt, ap_cp);
		ret[prn_size] = 0;
	}
	va_end(ap_cp);
	va_end(ap);
	return ret;
}



static inline silc_bool silc_str_chk_name_str(char const *in_str)
{
    uint32_t i;
    uint32_t len_max = strlen(in_str);

    for (i = 0; i < len_max; i++)
    {
        if ( (in_str[i] >= 0x61) && (in_str[i] <= 0x7A) )
        {
            /* in_str[i] is [a-z] */
        	continue;
        }
        else if ( (in_str[i] >= 0x41) && (in_str[i] <= 0x5a) )
        {
            /* in_str[i] is [A-Z] */
        	continue;
        }
        else if ( (in_str[i] >= 0x30) && (in_str[i] <= 0x39) )
        {
            /* in_str[i] is [0-9] */
        	continue;
        }
        else if ( (in_str[i] == '.') || (in_str[i] == '_') || (in_str[i] == '-') )
        {
        	continue;
        }
        else
        {
            return silc_false;
        }
    }
    return silc_true ;
}



#ifdef __cplusplus
}
#endif

#endif /* SILC_STR_H_ */
