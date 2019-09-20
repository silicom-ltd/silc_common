/*
 * silc_htbl.h
 *
 *  Created on: Nov 25, 2010
 *      Author: jeff_zheng
 *   Copyright: NetPerform Technology
 */

#ifndef SILC_HTBL_H_
#define SILC_HTBL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "silc_common/silc_list.h"

typedef uint32_t (*silc_htbl_hash_callback)(const void* p_key);

typedef int (*silc_htbl_cmp_callback)(const void* p_key1, const void* p_key2);

typedef struct silc_htbl_s
{
	silc_mcache*	hash_node_cache;
	uint32_t	hash_bit;
	uint32_t	hash_mask;
	uint32_t	hash_cnt;
	uint32_t	size_key;
	uint32_t    size_elem;
	silc_htbl_hash_callback hash_cbf;
	silc_htbl_cmp_callback cmp_cbf;
	silc_list*	hash_array;
}silc_htbl;

typedef struct silc_htbl_node_s
{
	silc_list_node hash_node;
	void*		 p_key;
	void*		 p_elem;
}silc_htbl_node;

#define SILC_HTBL_HASH_SIZE(bits) 	(1<<(bits))
#define SILC_HTBL_HASH_MASK(bits)		((1<<(bits)) -1)
#define SILC_HTBL_HASH_SLOT(bits, hval)  ((hval)& SILC_HTBL_HASH_MASK(bits))


static inline silc_htbl* silc_htbl_create(uint32_t hash_bits, silc_htbl_hash_callback hash_func, silc_htbl_cmp_callback cmp_func, uint32_t est_size)
{
	uint32_t node_size = sizeof(silc_htbl_node);
	uint32_t hash_size = SILC_HTBL_HASH_SIZE(hash_bits);

	silc_htbl* ret_tbl = (silc_htbl*)malloc(sizeof(silc_htbl));

	if(est_size==0)
	{
		est_size = 4096/node_size;
	}

	ret_tbl->hash_node_cache = silc_mcache_create((char*)"hbl",node_size,0,est_size);
	if(ret_tbl->hash_node_cache==NULL)
	{
		free(ret_tbl);
		return NULL;
	}
	ret_tbl->hash_cbf = hash_func;
	ret_tbl->cmp_cbf = cmp_func;
	ret_tbl->hash_bit = hash_bits;
	ret_tbl->hash_cnt = hash_size;
	ret_tbl->hash_mask = SILC_HTBL_HASH_MASK(hash_bits);
	ret_tbl->hash_array = (silc_list*)malloc(sizeof(silc_list)*hash_size);
	uint32_t loop;
	for(loop=0;loop<hash_size;loop++)
	{
		silc_list_init(ret_tbl->hash_array+loop);
	}
	return ret_tbl;
}

static inline int silc_htbl_destroy(silc_htbl*  htbl)
{
	if(htbl != NULL)
	{
		if(htbl->hash_array != NULL)
		{
			free(htbl->hash_array);
		}
		if(htbl->hash_node_cache)
		{
			silc_mcache_destroy(htbl->hash_node_cache);
		}
	free(htbl);
	}
	return 0;
}

static inline int silc_htbl_insert(silc_htbl* htbl, void* key, void* elem)
{
	uint32_t hval = htbl->hash_cbf(key);
	uint32_t hslot = SILC_HTBL_HASH_SLOT(htbl->hash_bit, hval);
	silc_list* head = htbl->hash_array + hslot;
	
	silc_htbl_node* p_node;

	silc_list_for_each_entry(p_node, head, hash_node)
	{
		if(htbl->cmp_cbf(key, (void*)(p_node+1))==0)
			return -1;
	}
	p_node = (silc_htbl_node*)silc_mcache_alloc(htbl->hash_node_cache);
	p_node->p_key = key;
	p_node->p_elem = elem;
	silc_list_add_tail(&p_node->hash_node, head);
	return 0;
}

static inline silc_htbl_node* silc_htbl_get_node(silc_htbl* htbl, const void* key)
{
	uint32_t hval = htbl->hash_cbf(key);
	uint32_t hslot = SILC_HTBL_HASH_SLOT(htbl->hash_bit, hval);
	silc_list* head = htbl->hash_array + hslot;
	
	silc_htbl_node* p_node;

	silc_list_for_each_entry(p_node, head, hash_node)
	{
		if(htbl->cmp_cbf(key, p_node->p_key)==0)
			return p_node;
	}

	return NULL;
}

static inline void* silc_htbl_get_elem(silc_htbl* htbl, const void* key)
{
	uint32_t hval = htbl->hash_cbf(key);
	uint32_t hslot = SILC_HTBL_HASH_SLOT(htbl->hash_bit, hval);
	silc_list* head = htbl->hash_array + hslot;

	silc_htbl_node* p_node;

	silc_list_for_each_entry(p_node, head, hash_node)
	{
		if(htbl->cmp_cbf(key, p_node->p_key)==0)
			return p_node->p_elem;
	}

	return NULL;
}


static inline int silc_htbl_del_by_node(silc_htbl* htbl, silc_htbl_node* p_node)
{
	silc_list_del(&p_node->hash_node);
	return 0;
}

static inline int silc_htbl_del_by_key(silc_htbl* htbl, void* key)
{
	silc_htbl_node* p_node = silc_htbl_get_node(htbl, key);
	if(p_node==NULL)
		return -1;
	return silc_htbl_del_by_node(htbl, p_node);
}


extern uint64_t silc_htbl_jenkins_hash(
		register const uint8_t *k,  /* the key */
		register uint64_t  length);   /* the length of the key */

#define silc_htbl_for_each_pair( p_usr_key, p_usr_elem, p_htbl, key_type, elem_type) \
	uint32_t hashloop__;				\
	for(hashloop__=0; hashloop__<(p_htbl)->hash_cnt;hashloop__++)	\
	{ \
		silc_htbl_node* p_node__, *p_node_tmp__; \
		silc_list* p_list__ = (p_htbl)->hash_array + (hashloop__&((p_htbl)->hash_mask)); \
		silc_list_for_each_entry_safe(p_node__, p_node_tmp__, p_list__, hash_node) \
		{ \
			p_usr_key = (key_type*)(p_node__->p_key); \
			p_usr_elem = (elem_type*)(p_node__->p_elem);
#define silc_htbl_for_each_pair_end()  } }

#define silc_htbl_for_each_entry(p_elem_, p_htbl, elem_type) \
	uint32_t hashloop__;				\
	for(hashloop__=0; hashloop__<(p_htbl)->hash_cnt;hashloop__++)	\
	{ \
		silc_htbl_node* p_node__, *p_node_tmp__; \
		silc_list* p_list__ = (p_htbl)->hash_array + (hashloop__&((p_htbl)->hash_mask)); \
		silc_list_for_each_entry_safe(p_node__, p_node_tmp__, p_list__, hash_node) \
		{ \
			p_elem_ = (elem_type*)(p_node__->p_elem);

#define silc_htbl_for_each_entry_end()  } }

#define silc_htbl_type_def_1(name, key_type, elem_type) 		\
typedef uint32_t (* silc_htbl_ ## name ## _hash_callback) (const key_type* p_key); \
typedef int (*silc_htbl_ ## name ## _cmp_callback)(const key_type* p_key1, const key_type* p_key2); \
static inline silc_htbl* silc_htbl_ ## name ## _create(uint32_t hash_bits, \
				silc_htbl_##name##_hash_callback hash_func, silc_htbl_##name##_cmp_callback cmp_func, uint32_t est_size) \
{ return silc_htbl_create(hash_bits, (silc_htbl_hash_callback) hash_func, (silc_htbl_cmp_callback)cmp_func, est_size); }\
	\

#define silc_htbl_type_def_2(name, key_type, elem_type)		\
static inline int silc_htbl_ ## name ## _insert(silc_htbl* htbl, key_type const * p_key, elem_type const * p_elem) \
{	return silc_htbl_insert(htbl, (void*)p_key, (void*)p_elem); } \
	\
static inline elem_type* silc_htbl_##name##_get_elem(silc_htbl* htbl, key_type const * p_key) \
{	return (elem_type*)silc_htbl_get_elem(htbl, (void*)p_key); } \
	\
static inline silc_htbl_node* silc_htbl_##name##_get_node(silc_htbl* htbl, key_type const * p_key) \
{	return silc_htbl_get_node(htbl, (void*)p_key); } \
	\
static inline int silc_htbl_##name##_del_by_node(silc_htbl* htbl, silc_htbl_node* p_node) \
{ return silc_htbl_del_by_node(htbl, p_node); } \
	\
static inline int silc_htbl_##name##_del_by_key(silc_htbl* htbl, key_type* p_key) \
{ return silc_htbl_del_by_key(htbl, (void*)p_key); } \


#define silc_htbl_type_def(name, key_type, elem_type) 		\
		silc_htbl_type_def_1(name, key_type, elem_type)		\
		silc_htbl_type_def_2(name, key_type, elem_type)

#define silc_htbl_type_def_cstr(name, elem_type) 		\
typedef uint32_t (* silc_htbl_ ## name ## _hash_callback) (const silc_str p_key); \
typedef int (*silc_htbl_ ## name ## _cmp_callback)(const silc_str p_key1, const silc_str p_key2); \
static inline silc_htbl* silc_htbl_ ## name ## _create(uint32_t hash_bits, uint32_t est_size) \
{ return silc_htbl_create(hash_bits, (silc_htbl_hash_callback) silc_htbl_default_cstr_hash, \
					(silc_htbl_cmp_callback)silc_htbl_default_cstr_comp, est_size); }\

#define silc_htbl_cstr_type_def(name, elem_type) \
		silc_htbl_type_def_cstr(name, elem_type) \
		silc_htbl_type_def_2(name, char, elem_type)

extern uint32_t silc_htbl_default_cstr_hash(silc_cstr p_str);

extern int silc_htbl_default_cstr_comp(const silc_cstr p_str1, const silc_cstr p_str2);


#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <string>

extern uint32_t silc_htbl_default_cxxstr_hash(std::string* p_str);
extern int silc_htbl_default_cxxstr_comp(const std::string* p_str1, const std::string* p_str2);

#define silc_htbl_type_def_cxxstr(name, elem_type) 		\
typedef uint32_t (* silc_htbl_ ## name ## _hash_callback) (const string* p_key); \
typedef int (*silc_htbl_ ## name ## _cmp_callback)(const string* p_key1, const string* p_key2); \
static inline silc_htbl* silc_htbl_ ## name ## _create(uint32_t hash_bits, uint32_t est_size) \
{ return silc_htbl_create(hash_bits, (silc_htbl_hash_callback) silc_htbl_default_cxxstr_hash, \
					(silc_htbl_cmp_callback)silc_htbl_default_cxxstr_comp, est_size); }\

#define silc_htbl_cxxstr_type_def(name, elem_type) \
		silc_htbl_type_def_cxxstr(name, elem_type) \
		silc_htbl_type_def_2(name, string, elem_type)

#endif


#endif /* SILC_HTBL_H_ */
