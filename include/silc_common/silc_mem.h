/*
 * silc_mem.h
 *
 *  Created on: Nov 24, 2010
 *      Author: jeff_zheng
 *   Copyright: NetPerform Technology
 */

#ifndef SILC_MEM_H_
#define SILC_MEM_H_

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @defgroup silc_mem silc_mem
 * @brief A memory allocator lib
 * @ingroup silc_common
 *
 * A memory allocator lib. Like linux kernel memory cache, it helps allocating objects of the same size
 */

extern uint64_t g_silc_mcache_global_mem_limit;
extern uint64_t g_silc_mcache_global_mem_current;
#ifdef POWERPC_FSL_LINUX
extern pthread_mutex_t g_silc_mcache_global_mem_lock;
#endif

#define SILC_MEM_PAGE_BITS	((uintptr_t)12)
#define SILC_MEM_PAGE_SIZE	((uintptr_t)1<<SILC_MEM_PAGE_BITS)
#define SILC_MEM_PAGE_MASK	(~(SILC_MEM_PAGE_SIZE-1))
#define SILC_MEM_PAGE_ALIGN_BASE(ptr, type)		((type)( ((uintptr_t)(ptr)) & SILC_MEM_PAGE_MASK))
#define SILC_MEM_PAGE_ALIGN_END(ptr, type)		((type)( (((uintptr_t)(ptr))+ (SILC_MEM_PAGE_SIZE-1)) & SILC_MEM_PAGE_MASK))


#undef SILC_MEM_USE_MALLOC
#ifdef SILC_MEM_USE_MALLOC
#define silc_mcache_alloc silc_mcache_alloc_libc
#define silc_mcache_free silc_mcache_free_libc
#else
#define silc_mcache_alloc silc_mcache_alloc_real
#define silc_mcache_free silc_mcache_free_real

#endif

#define SILC_PTR_OFFSET(base, off, new_type) \
	((new_type*)(((char*)(base)) + (off)))



struct silc_mcache_s;
typedef int (*silc_mem_ctor_callback)(void* p_obj, struct silc_mcache_s * mcache, void* p_usr_arg);
typedef int (*silc_mem_dtor_callback)(void* p_obj, struct silc_mcache_s * mcache, void* p_usr_arg);


typedef struct silc_mcache_s
{
	char		name[16];			//name of this memory cache

	uint32_t	align;				//controls alignment of allocated memory, 1 or 0 means no alignment requirement
	uint32_t	elem_size;			//size of element for each allocation
	uint32_t 	grow;				//this indicate how shall we grow this memory cache, when more objects are requested
	uint32_t 	mblk_size;			//size of each memory block, each memory block is a direct allocation from libc malloc
									//the memoby block is then devided into small pieces of objects, and put onto the
									//free list of the memory block.
	silc_list		mblk_list;			//list of memory blocks.
	silc_list		mblk_free_list;		//list of memory blocks that has free memory objects for allocation
	silc_list		mblk_full_list;		//list of memory blocks that has no memory objects for allocation
	uint32_t	mblk_cnt;			//total number of memory blocks
#ifdef POWERPC_FSL_LINUX
	pthread_mutex_t m_lock;
#endif


//	silc_mem_ctor_callback ctor;
//	silc_mem_dtor_callback dtor;
//	void*		p_usr_arg;
}silc_mcache;


typedef struct silc_mcache_blk_s
{
	silc_mcache*		mcache;			// the memory cache this memory block belongs to
	silc_list_node	list_node;		// list node used to put ton mblk_list
	silc_list_node	usage_node;		//list node used to put ton mblk_free_list or mblk_full_list
	uint32_t		mblk_id;		//mblock id, not used
	silc_slist 		free_list;		//list of free object available for allocation
	uint32_t 		free_count;		//number fo free object on the free_list
	uint32_t 		total_count;	//total number of object belongs to this mblk
}silc_mcache_blk;

typedef struct silc_mcache_node_s
{
	silc_slist_node	snode;			//slist node to be chained on mblk->free_list
	silc_mcache_blk*	blk;			//mblk that this memory object belongs to
}silc_mcache_node;

#define SILC_MCACHE_DEFAULT_OBJ_GROW	256

#define SILC_MCACHE_ALIGN(v, align)		((((uintptr_t)(v)+(align)-1)/(align))*(align))

static inline void silc_mcache_set_global_mem_limit(uint64_t new_global_mem_limit)
{
	g_silc_mcache_global_mem_limit = new_global_mem_limit;
}
static inline uint32_t silc_mcache_item_size(silc_mcache* mcache)
{
	return mcache->elem_size - sizeof(silc_mcache_node);
}
static inline uint32_t silc_mcache_item_size_real(silc_mcache* mcache)
{
	return mcache->elem_size;
}
/**
 * @ingroup silc_mem
 * @brief internal function, to add and intialize a new mblk when there's no free object to allocate
 *
 * @param[in] p_mcache the mcache to extend
 * @return 0 for success. -1 for failure
 *
 * this function calls malloc and intialize a new mblk and slice it to smaller memory objects and chain them
 * in the mblk->free_list
 **/
static inline int silc_mcache_add_new_mblk(silc_mcache* mcache)
{
	uintptr_t blk_real_size = sizeof(silc_mcache_blk) + mcache->mblk_size + (mcache->align)*2;

	if(g_silc_mcache_global_mem_current > g_silc_mcache_global_mem_limit)
		return -1;
#ifdef POWERPC_FSL_LINUX
	pthread_mutex_lock(&g_silc_mcache_global_mem_lock);
	g_silc_mcache_global_mem_current += blk_real_size;
	pthread_mutex_unlock(&g_silc_mcache_global_mem_lock);
#else
	__sync_fetch_and_add(&g_silc_mcache_global_mem_current, blk_real_size);
#endif

	silc_mcache_blk* p_blk = (silc_mcache_blk*)malloc(blk_real_size);
	if(p_blk==NULL)
		return -1;
	memset(p_blk, 0, sizeof(*p_blk));
	uintptr_t blk_end = ((uintptr_t)p_blk) + blk_real_size;

	uintptr_t elem_align_start = SILC_MCACHE_ALIGN(((uintptr_t)(p_blk+1))+sizeof(silc_mcache_node), mcache->align);
	silc_mcache_node* p_node_start = (silc_mcache_node*)(elem_align_start - sizeof(silc_mcache_node));
	p_blk->mcache = mcache;

	SILC_SLIST_INIT(p_blk->free_list);

	while( (((uintptr_t)p_node_start)+(uintptr_t)(mcache->elem_size)) < blk_end)
	{
		p_node_start->blk = p_blk;
		silc_slist_add_tail(&(p_node_start->snode), &(p_blk->free_list));
		p_blk->total_count ++;
		p_blk->free_count ++;
		p_node_start = (silc_mcache_node*)(((char*)p_node_start)+mcache->elem_size);
	}

	mcache->mblk_cnt++;
	silc_list_add_tail(&(p_blk->list_node), &(mcache->mblk_list));
	silc_list_add_tail(&(p_blk->usage_node), &(mcache->mblk_free_list));
	return 0;
}


/**
 * @ingroup silc_mem
 * @brief create a memory cache
 *
 * @param[in] name 			name of the mcache
 * @param[in] elem_size 	size of the object for each allocation
 * @param[in] align 		aligment requirement fot the memory object
 * @param[in] grow 			grow factor of the memory cache. this decide the intialize size,
 *                          and how the mcache should be growing. This does affect the number of malloc calls
 *                          a wise value if important. passing 0, will get the mcache to use the default value
 *                          which would be too small for large amount of allocation.
 * @return the created memory cache. NULL for failure
 *
 **/
static inline silc_mcache* silc_mcache_create(char* name, uint32_t elem_size, uint32_t align, uint32_t grow)
//		void* p_usr_arg, silc_mem_ctor_callback ctor, silc_mem_dtor_callback dtor)
{
	silc_mcache* ret_cache = (silc_mcache*)malloc(sizeof(*ret_cache));
	if(ret_cache==NULL)
	{
		SILC_ERR("Failed to create memory cache %s", name);
		return NULL;
	}
	memset(ret_cache, 0, sizeof(*ret_cache));
	if(grow==0)
		grow = 256;
	strncpy(ret_cache->name, name, sizeof(ret_cache->name)-1);
	ret_cache->name[sizeof(ret_cache->name)-1] = 0;

	ret_cache->grow = grow;
	ret_cache->elem_size = elem_size;
	if(align==0)
		align = 1;
	ret_cache->align = align;
	ret_cache->elem_size = ((sizeof(silc_mcache_node)  + ret_cache->elem_size + align -1)/align) *align;
	ret_cache->mblk_size = ret_cache->elem_size * ret_cache->grow;
//	ret_cache->ctor = ctor;
//	ret_cache->dtor = dtor;
//	ret_cache->p_usr_arg = p_usr_arg;

	SILC_LIST_INIT(ret_cache->mblk_list);
	SILC_LIST_INIT(ret_cache->mblk_free_list);
	SILC_LIST_INIT(ret_cache->mblk_full_list);

	silc_mcache_add_new_mblk(ret_cache);

	return ret_cache;
}

/**
 * @ingroup silc_mem
 * @brief destory a previosly create mcache
 *
 * @param[in] mcache 		the mcache to destroy
 * @return
 *
 **/
static inline void silc_mcache_destroy(silc_mcache* mcache)
{
	silc_mcache_blk* blk, *blk_tmp;
	if(mcache == NULL)
		return;
	silc_list_for_each_entry_safe(blk, blk_tmp, &(mcache->mblk_list), list_node)
	{
		silc_list_del(&(blk->list_node));
		memset(blk,0,sizeof(*blk));
		free(blk);
	}
	free(mcache);
	return;
}

/**
 * @ingroup silc_mem
 * @brief main allocation function
 *
 * @param[in] mcache 		the mcache to destroy
 * @return
 *
 **/
static inline void* silc_mcache_alloc_real(silc_mcache* mcache)
{
	silc_mcache_blk* blk;
	silc_mcache_node* ret_node;
	void* ret;
	if(silc_list_empty(&(mcache->mblk_free_list)))
	{
		if(0!=silc_mcache_add_new_mblk(mcache))
			return NULL;
	}
	blk = silc_list_entry(mcache->mblk_free_list.next, silc_mcache_blk, usage_node);
	if(silc_slist_empty(&(blk->free_list))||blk->free_count == 0)
	{
		return NULL;
	}
	ret_node = silc_slist_entry(blk->free_list.first, silc_mcache_node, snode);
	silc_slist_del(&(ret_node->snode), NULL, &(blk->free_list));
	blk->free_count --;

	if(blk->free_count==0)
	{
		silc_list_del(&blk->usage_node);
		silc_list_add_tail(&blk->usage_node, &mcache->mblk_full_list);
	}

	ret = (void*)(ret_node+1);
//	if(mcache->ctor)
//		mcache->ctor(ret, mcache, mcache->p_usr_arg);

	return ret;
}

/**
 * @ingroup silc_mem
 * @brief libc allocation wrapper for valgrind debug
 *
 * @param[in] mcache 		the mcache to destroy
 * @return
 *
 **/
static inline void* silc_mcache_alloc_libc(silc_mcache* mcache)
{
	return malloc(mcache->elem_size-sizeof(silc_mcache_node));
}

/**
 * @ingroup silc_mem
 * @brief  libc allocation wrapper for valgrind debug
 *
 * @param[in] mcache 		the mcache to destroy
 * @return
 *
 **/
static inline int silc_mcache_free_libc(void* mem)
{
	free(mem);
	return 0;
}

/**
 * @ingroup silc_mem
 * @brief main free function
 *
 * @param[in] mcache 		the mcache to destroy
 * @return
 *
 **/
static inline int silc_mcache_free_real(void* mem)
{
	silc_mcache_node* ret_node = ((silc_mcache_node*)mem)-1;
	silc_mcache_blk*	blk = ret_node->blk;
	silc_mcache* mcache = blk->mcache;

	if(ret_node->snode.next != NULL)
	{
		SILC_ERR("Failed to free memory\n");
		return -1;
	}
//	if(mcache->dtor)
//		mcache->dtor(mem, mcache, mcache->p_usr_arg);

	silc_slist_add_tail(&(ret_node->snode), &(blk->free_list));
	blk->free_count ++;
	if(blk->free_count==1)
	{//it was on the full list
		//move to mcache free list
		silc_list_del(&(blk->usage_node));
		silc_list_add_head(&(blk->usage_node), &(mcache->mblk_free_list));
	}
	else
	{
#if 1
		//resort mcache free list
		silc_list_node* next = blk->usage_node.next;
		silc_mcache_blk*	next_blk;
		silc_mcache_blk*	insert_after_blk = NULL;
		while(next!=(silc_list_node*)(&mcache->mblk_free_list))
		{
			next_blk = silc_list_entry(next, silc_mcache_blk, usage_node);
			if(next_blk->free_count>=blk->free_count)
			{
				break;
			}
			insert_after_blk = next_blk;
			next = next_blk->usage_node.next;
		}
		silc_list_del(&(blk->usage_node));

		if(insert_after_blk==NULL)
			silc_list_add_tail(&blk->usage_node, &mcache->mblk_free_list);
		else
			silc_list_add(&blk->usage_node, &insert_after_blk->usage_node);
#endif
	}
	return 0;
}



#ifdef __cplusplus
}
#endif

#endif /* SILC_MEM_H_ */
