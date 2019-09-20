/*
 * silc_array.h
 *
 *  Created on: Nov 23, 2010
 *      Author: jeff_zheng
 *   Copyright: NetPerform Technology
 */

#ifndef SILC_ARRAY_H_
#define SILC_ARRAY_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup silc_array silc_array
 * @brief a dynamic array implementation
 * @ingroup silc_common
 *
 * A dynamic array implementation, which focuses on performance, and quick allocation.
 */


#define SILC_ARRAY_INITIAL_BLK_COUNT	8
#define SILC_ARRAY_DEFAULT_BLK_SIZE	8096
#define SILC_ARRAY_DEFAULT_BLK_CNT_MIN	64
typedef char* silc_array_blk_ptr;

typedef struct silc_array_s
{
	uint32_t length;
	uint32_t elem_size;
	uint32_t blk_size_bytes;
	uint32_t blk_elem_cnt;
	uint32_t blk_count;
	silc_array_blk_ptr* blk_ptr_base;
}silc_array, *silc_array_id;

/**
 * @ingroup silc_array
 * @brief Destroy a stats accumulator
 *
 * @param[in] p_array 	        structure to destroy
 * @return 0 for success, non-zero for Error
 *
 * Destroy a array, free all resources.
 **/
static inline void silc_array_destroy(silc_array* p_array)
{
	uint32_t loop;
	if(p_array)
	{
		if(p_array->blk_ptr_base)
		{
			for(loop=0;loop<p_array->blk_count;loop++)
			{
				if(p_array->blk_ptr_base[loop])
				{
					free(p_array->blk_ptr_base[loop]);
					p_array->blk_ptr_base[loop] = NULL;
				}
			}
			free(p_array->blk_ptr_base);
		}
		free(p_array);
	}
}

/**
 * @ingroup silc_array
 * @brief create a silc_arra
 *
 * @param[in] elem_size 	        size of the the element to be stored in this array
 * @param[in] initial_size 	        Hint for the size of the array. can be 0;
 * @return 0 for success, non-zero for Error
 *
 * Create a array.
 **/
static inline silc_array* silc_array_create(uint32_t elem_size, uint32_t initial_size)
{
	uint32_t blk_elem_cnt, blk_size_bytes;
	uint32_t blk_cnt = SILC_ARRAY_INITIAL_BLK_COUNT;

	if(initial_size == 0)
	{/*no default giving, try guessing a good value*/
		blk_size_bytes = SILC_ARRAY_DEFAULT_BLK_SIZE;
		blk_elem_cnt = blk_size_bytes/elem_size;
		while(blk_elem_cnt<SILC_ARRAY_DEFAULT_BLK_CNT_MIN)
		{
			blk_size_bytes = blk_size_bytes*2;
			blk_elem_cnt = blk_size_bytes/elem_size;
		}
	}
	else
	{/*honor user request*/
		blk_elem_cnt = initial_size;
		blk_size_bytes = blk_elem_cnt * elem_size;
	}

	silc_array* p_ret = (silc_array*)malloc(sizeof(silc_array));
	if(p_ret==NULL)
	{
		return NULL;
	}
	memset(p_ret, 0, sizeof(*p_ret));

	p_ret->length = 0;
	p_ret->elem_size = elem_size;
	p_ret->blk_elem_cnt = blk_elem_cnt;
	p_ret->blk_count = blk_cnt;
	p_ret->blk_size_bytes = blk_size_bytes;
	p_ret->blk_ptr_base = (silc_array_blk_ptr*)malloc(sizeof(silc_array_blk_ptr)*blk_cnt);
	if(p_ret->blk_ptr_base == NULL)
	{
		silc_array_destroy(p_ret);
		return NULL;
	}
	memset(p_ret->blk_ptr_base, 0, sizeof(silc_array_blk_ptr)*blk_cnt);

	p_ret->blk_ptr_base[0] = (silc_array_blk_ptr)malloc(blk_size_bytes);
	if(p_ret->blk_ptr_base[0] == NULL)
	{
		silc_array_destroy(p_ret);
		return NULL;
	}
	return p_ret;
}

/**
 * @ingroup silc_array
 * @brief to ensure the specified array has enough space for the specified number of elements
 *
 * @param[in] array 	        array to operate on
 * @param[in] index 	        start index where new elements will be put
 * @param[in] count 	        number of elments needed
 * @return 0 for success, non-zero for Error
 *
 * Create a array.
 **/
static inline int silc_array_ensure_space(silc_array* p_array, uint32_t start_idx, uint32_t count)
{
	uint32_t blk_max = (start_idx + count)/p_array->blk_elem_cnt + 1;
	uint32_t loop;

	if(p_array->blk_count < blk_max)
	{
		uint32_t new_blk_count = p_array->blk_count << 1;
		while(new_blk_count < blk_max)
		{
			new_blk_count = new_blk_count << 1;
		}
		silc_array_blk_ptr* p_new_base = (silc_array_blk_ptr*)realloc(p_array->blk_ptr_base, sizeof(silc_array_blk_ptr)*new_blk_count);
		if(p_new_base==NULL)
			return -1;
		p_array->blk_ptr_base = p_new_base;
		memset(p_array->blk_ptr_base + p_array->blk_count,0,sizeof(silc_array_blk_ptr)*(new_blk_count-p_array->blk_count));
		p_array->blk_count = new_blk_count;
	}
	for(loop = start_idx/p_array->blk_elem_cnt; loop < blk_max; loop++)
	{
		if(p_array->blk_ptr_base[loop] == NULL)
		{
			p_array->blk_ptr_base[loop] = (silc_array_blk_ptr)malloc(p_array->blk_size_bytes);
			if(p_array->blk_ptr_base[loop] == NULL)
				return -1;
		}
	}
	return 0;
}

/**
 * @ingroup silc_array
 * @brief put one or more elements into the array, the data will be copied into the array
 *
 * @param[in] array 	        array to operate on
 * @param[in] index 	        start index where new elements will be put
 * @param[in] elem 	        	pionter to the start of the elements to be put into the array
 * @param[in] count 	        number of elments to put into the array
 * @return 0 for success, non-zero for Error
 *
 * put one or more elements into the array, the data will be copied into the array
 **/
static inline int silc_array_put(silc_array* p_array, uint32_t start_idx, const void* elem, uint32_t count)
{
	if(0!=silc_array_ensure_space(p_array, start_idx, count))
		return -1;
	uint32_t cp_start_blk_idx    =  start_idx/p_array->blk_elem_cnt;
	uint32_t cp_start_blk_elem_used = start_idx%p_array->blk_elem_cnt;
	uint32_t cp_start_blk_elem_free = p_array->blk_elem_cnt - cp_start_blk_elem_used;
	uint32_t copied = 0;
	uint32_t to_copy = count < cp_start_blk_elem_free ? count : cp_start_blk_elem_free;
	uint32_t cp_start_blk_byte_start = (cp_start_blk_elem_used)*p_array->elem_size;

	while(1)
	{
		silc_array_blk_ptr blk_base = p_array->blk_ptr_base[cp_start_blk_idx];

		memcpy(blk_base + cp_start_blk_byte_start,
				((char*)elem) + (p_array->elem_size*copied), to_copy*p_array->elem_size);
		cp_start_blk_byte_start = 0; //we copy from byte 0 for subsequent blocks

		copied += to_copy;
		to_copy = count - copied;
		if(to_copy == 0)
			break;
		if(to_copy > p_array->blk_elem_cnt)
			to_copy = p_array->blk_elem_cnt;
		cp_start_blk_idx++;
	}
	p_array->length = start_idx + count;
	return 0;
}

/**
 * @ingroup silc_array
 * @brief put elements to the current end of the array
 *
 * @param[in] array 	        array to operate on
 * @param[in] elem 	        	pionter to the start of the elements to be put into the array
 * @param[in] count 	        number of elments to put into the array
 * @return 0 for success, non-zero for Error
 *
 * put elements to the current end of the array, increase the current size of the array by count
 **/
static inline int silc_array_put_tail(silc_array* p_array, const void* elem, uint32_t count)
{
	return silc_array_put(p_array, p_array->length, elem, count);
}

/**
 * @ingroup silc_array
 * @brief return a pointer to an stored element
 *
 * @param[in] array 	        array to operate on
 * @param[in] index 	        the index of the element to return
 * @return 0 for success, non-zero for Error
 *
 * return a pointer to an stored element.
 **/
static inline void* silc_array_get_quick(silc_array* p_array, uint32_t start_idx)
{
	uint32_t blk_idx = start_idx/p_array->blk_elem_cnt;
	uint32_t blk_off = (start_idx%p_array->blk_elem_cnt)*p_array->elem_size;

	if(blk_idx>=p_array->blk_count)
		return NULL;
	silc_array_blk_ptr blk_base = p_array->blk_ptr_base[blk_idx];
	if(blk_base==NULL)
		return NULL;
	return (void*)(blk_base + blk_off);
}

static inline int silc_array_get_copy(silc_array* p_array, uint32_t start_idx, void* elem)
{
	uint32_t blk_idx = start_idx/p_array->blk_elem_cnt;
	uint32_t blk_off = (start_idx%p_array->blk_elem_cnt)*p_array->elem_size;

	if(blk_idx>=p_array->blk_count)
		return -1;
	silc_array_blk_ptr blk_base = p_array->blk_ptr_base[blk_idx];
	if(blk_base==NULL)
		return -1;
	memcpy(elem, (void*)(blk_base + blk_off), p_array->elem_size);
	return 0;
}

static inline int silc_array_del_last(silc_array* p_array, void* elem)
{
	uint32_t end = p_array->length;
	if(end)
	{
		void* del_elem_ptr = silc_array_get_quick(p_array, end-1);
		if(del_elem_ptr)
		{
			if(elem)
			{
				memcpy(elem, del_elem_ptr, p_array->elem_size);
			}
			memset(del_elem_ptr, 0, p_array->elem_size);
			p_array->length--;
			return 0;
		}
	}

	return -1;
}

/**
 * @ingroup silc_array
 * @brief return the current size of the array
 *
 * @param[in] array 	        array to operate on
 * @return 0 for success, non-zero for Error
 *
 *  return the current size of the array.
 **/
static inline uint32_t silc_array_get_length(silc_array* p_array)
{
	return p_array->length;
}

/**
 * @ingroup silc_array
 * @brief shrink an array to the specified size
 *
 * @param[in] array 	        array to operate on
 * @return 0 for success, non-zero for Error
 *
 *  shrink an array to the specified size.
 **/
static inline int silc_array_shrink(silc_array* p_array, uint32_t new_size)
{
	uint32_t blk_free_start, blk_free_end;
	if(p_array->length < new_size)
		return -1;
	blk_free_start = (new_size/p_array->blk_elem_cnt) + 1;
	blk_free_end = (p_array->length/p_array->blk_elem_cnt);

	while(blk_free_start<=blk_free_end)
	{
		if(p_array->blk_ptr_base[blk_free_start])
		{
			free(p_array->blk_ptr_base[blk_free_start]);
			p_array->blk_ptr_base[blk_free_start] = NULL;
		}
		blk_free_start++;
	}
	p_array->length = new_size;
	return 0;
}


static inline int silc_array_merge(silc_array* dest, const silc_array* other)
{
	if(dest->elem_size!=other->elem_size)
	{
		//can't merge arrays with different element size
		return -1;
	}
	uint32_t merged=0, to_copy;
	uint32_t loop;
	for(loop=0; loop<other->blk_count && merged < other->length; loop++)
	{
		void* blk_start = other->blk_ptr_base[loop];
		to_copy = other->length - merged;
		if(to_copy > other->blk_elem_cnt)
			to_copy = other->blk_elem_cnt;
		if(blk_start!=NULL)
		{
			silc_array_put_tail(dest, blk_start, to_copy);
		}
		merged += to_copy;
	}
	return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* SILC_ARRAY_H_ */
