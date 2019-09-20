/*
 * silc_ring_buffer.h
 *
 *  Created on: Nov 22, 2010
 *      Author: jeff_zheng
 *   Copyright: NetPerform Technology
 */

#ifndef SILC_RING_BUFFER_H_
#define SILC_RING_BUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*silc_rng_dtor_cbf)(void* p_elem);
typedef struct silc_rng_buf_s
{
	void* p_buf;
	volatile uint32_t idx_first;
	volatile uint32_t idx_last;
	uint32_t total_count;
	uint32_t elem_size;
	silc_rng_dtor_cbf dtor;
}silc_rng_buf, * silc_rng_buf_id;



static inline silc_rng_buf_id silc_rng_buf_create(uint32_t elem_size, uint32_t size, silc_rng_dtor_cbf dtor)
{
	if(elem_size ==0 || size==0)
		return NULL;
	silc_rng_buf_id rng_id = (silc_rng_buf_id)malloc(sizeof(silc_rng_buf));
	if(rng_id==NULL)
		return NULL;
	memset(rng_id, 0, sizeof(*rng_id));
	rng_id->elem_size = elem_size;
	rng_id->total_count = size+1;
	rng_id->p_buf = calloc(rng_id->elem_size,rng_id->total_count);
	if(rng_id->p_buf==NULL)
	{
		free(rng_id);
		return NULL;
	}
	return rng_id;
}

static inline int silc_rng_buf_destroy(silc_rng_buf_id rng_id)
{
	if(rng_id==NULL)
		return -1;
	free(rng_id->p_buf);
	free(rng_id);
	return 0;
}

static inline uint32_t silc_rng_buf_get_size(silc_rng_buf_id rng_id)
{
	return rng_id->total_count - 1;
}

static inline uint32_t silc_rng_buf_get_length(silc_rng_buf_id rng_id)
{
	if(rng_id->idx_first > rng_id->idx_last)
		return rng_id->total_count + rng_id->idx_last - rng_id->idx_first;
	else
		return rng_id->idx_last - rng_id->idx_first;
}


static inline uint32_t silc_rng_buf_get_free_count(silc_rng_buf_id rng_id)
{
	if(rng_id->idx_first > rng_id->idx_last)
		return rng_id->idx_first - rng_id->idx_last - 1;
	else
		return rng_id->total_count -rng_id->idx_last + rng_id->idx_first -1;

}

static inline void silc_rng_buf_data_cpy_from_usr(silc_rng_buf_id rng_id, uint32_t cp_start, const void* p_cp_data,
										uint32_t usr_data_offset, uint32_t cp_count)
{
	memcpy(((char*)rng_id->p_buf) + cp_start*rng_id->elem_size,
			((char*)p_cp_data)+usr_data_offset*rng_id->elem_size,
			cp_count*rng_id->elem_size);

}

static inline int silc_rng_buf_push(silc_rng_buf_id rng_id, uint8_t const *p_element, uint32_t count)
{
	uint32_t copied = 0, cp_cnt;

	if (silc_rng_buf_get_free_count(rng_id) < count)
	{
		return -1;
	}

	cp_cnt = count;
	if(rng_id->idx_last >= rng_id->idx_first)
	{
		/* tmp_size is size of p_data copied to end of buffer. */
		cp_cnt = rng_id->total_count - rng_id->idx_last;
		if(cp_cnt > count)
			cp_cnt = count;
	}
	while(cp_cnt)
	{
		silc_rng_buf_data_cpy_from_usr(rng_id, rng_id->idx_last, p_element, copied, cp_cnt);
		copied += cp_cnt;
		rng_id->idx_last += cp_cnt;
		if(rng_id->idx_last==rng_id->total_count)
			rng_id->idx_last = 0;
		cp_cnt = count-copied;
	};

	return 0;
}

static inline void silc_rng_buf_data_cpy_to_usr(silc_rng_buf_id rng_id, uint32_t cp_start, const void* p_cp_dst,
										uint32_t usr_data_offset, uint32_t cp_count)
{
	memcpy( ((char*)p_cp_dst)+usr_data_offset*rng_id->elem_size,
			((char*)rng_id->p_buf) + cp_start*rng_id->elem_size,
			cp_count*rng_id->elem_size);

}

static inline int silc_rng_buf_peak(silc_rng_buf_id rng_id, uint32_t start_offset, void* p_usr_buf, uint32_t read_cnt)
{
	uint32_t curr_length = silc_rng_buf_get_length(rng_id);

	if((start_offset+read_cnt)>curr_length)
		return -1;

	uint32_t cp_cnt = 0;
	uint32_t actual_start = start_offset + rng_id->idx_first;

	if(actual_start>=rng_id->total_count)
		actual_start -= rng_id->total_count;

	if(actual_start>rng_id->idx_last)
	{
		cp_cnt = rng_id->total_count - actual_start;
		if(cp_cnt>read_cnt)
			cp_cnt = read_cnt;
		silc_rng_buf_data_cpy_to_usr(rng_id, actual_start, p_usr_buf, 0, cp_cnt);
		read_cnt -= cp_cnt;
		actual_start += cp_cnt;
		if(actual_start>=rng_id->total_count)
			actual_start = 0;
	}

	// copy the rest, if there's anything left
	if(read_cnt)
	{
		silc_rng_buf_data_cpy_to_usr(rng_id, actual_start, p_usr_buf, cp_cnt, read_cnt);
	}
	return 0;
}

static inline int silc_rng_buf_push_force(silc_rng_buf_id rng_id, const void* p_push_data,  void* p_pop_buf)
{
	if(silc_rng_buf_get_free_count(rng_id)==0)
	{
		if(p_pop_buf)
			silc_rng_buf_data_cpy_to_usr(rng_id, rng_id->idx_first, p_pop_buf, 0, 1);
		rng_id->idx_first ++;
		if(rng_id->idx_first==rng_id->total_count)
			rng_id->idx_first = 0;
	}
	silc_rng_buf_data_cpy_from_usr(rng_id, rng_id->idx_last, p_push_data, 0, 1);
	rng_id->idx_last++;
	if(rng_id->idx_last == rng_id->total_count)
		rng_id->idx_last = 0;
	return 0;
}


static inline int silc_rng_buf_pop(silc_rng_buf_id rng_id, void* p_pop_buf, uint32_t read_cnt)
{
	uint32_t used_cnt, copied = 0;
	uint32_t cp_cnt;

	used_cnt = silc_rng_buf_get_length(rng_id);

	if(used_cnt < read_cnt)
	{
		return -1;
	}

	cp_cnt = read_cnt;
	if(rng_id->idx_last<rng_id->idx_first)
	{
		cp_cnt = rng_id->total_count - rng_id->idx_first;
		if(cp_cnt>read_cnt)
			cp_cnt = read_cnt;
	}
	while(cp_cnt)
	{
		if(p_pop_buf)
		{
			silc_rng_buf_data_cpy_to_usr(rng_id, rng_id->idx_first, p_pop_buf, copied, cp_cnt);
		}
		copied += cp_cnt;
		rng_id->idx_first += cp_cnt;
		if(rng_id->idx_first >= rng_id->total_count)
			rng_id->idx_first = 0;
		cp_cnt = read_cnt - copied;
	}
	return 0;
}

static inline void silc_rng_buf_clear(silc_rng_buf_id rng_id)
{
	if(rng_id->dtor)
	{
		while(rng_id->idx_first!=rng_id->idx_last)
		{
			rng_id->dtor((uint8_t*)(rng_id->p_buf) + (rng_id->idx_first* rng_id->elem_size));
			rng_id->idx_first++;
			if(rng_id->idx_first==rng_id->total_count)
				rng_id->idx_first = 0;
		}
	}
	else
	{
		rng_id->idx_first = rng_id->idx_last;
	}
}


#define silc_rng_buf_type_def(name, elem_type) 		\
typedef void (*silc_rng_buf ## name ## _dtor)(elem_type* p_elem); \
static inline silc_rng_buf* silc_rng_buf_ ## name ## _create( uint32_t size, silc_rng_buf ## name ## _dtor dtor) \
{ return silc_rng_buf_create(sizeof(elem_type), size, (silc_rng_dtor_cbf)dtor); }\
	\
static inline int silc_rng_buf_ ## name ## _destroy(silc_rng_buf* rngbuf) \
{	return silc_rng_buf_destroy(rngbuf); } \
	\
static inline uint32_t silc_rng_buf_##name##_get_size(silc_rng_buf* rngbuf) \
{	return silc_rng_buf_get_size(rngbuf); } \
	\
static inline uint32_t silc_rng_buf_##name##_get_length(silc_rng_buf* rngbuf) \
{	return silc_rng_buf_get_length(rngbuf); } \
	\
static inline uint32_t silc_rng_buf_##name##_get_free_count(silc_rng_buf* rngbuf) \
{	return silc_rng_buf_get_free_count(rngbuf); } \
	\
static inline int silc_rng_buf_##name##_push(silc_rng_buf* rngbuf, elem_type const * p_data, uint32_t count) \
{	return silc_rng_buf_push(rngbuf, (uint8_t*)p_data, count); } \
	\
static inline int silc_rng_buf_##name##_push_force(silc_rng_buf* rngbuf, elem_type const * p_data, elem_type* p_pop_buf) \
{	return silc_rng_buf_push_force(rngbuf, (uint8_t*)p_data, p_pop_buf); } \
	\
static inline int silc_rng_buf_##name##_pop(silc_rng_buf* rngbuf, elem_type* p_pop_buf, uint32_t pop_size) \
{ return silc_rng_buf_pop(rngbuf, p_pop_buf, pop_size); } \
\
static inline int silc_rng_buf_##name##_peak(silc_rng_buf_id rng_id, uint32_t start_offset, elem_type* p_usr_buf, uint32_t read_cnt) \
{ return silc_rng_buf_peak(rng_id, start_offset, p_usr_buf, read_cnt); }



#ifdef __cplusplus
}
#endif

#endif /* SILC_RING_BUFFER_H_ */
