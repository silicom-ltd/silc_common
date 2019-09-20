/*
 * silc_thread.h
 *
 *  Created on: OCT 21, 2011
 *      Author: jeff.zheng
 */

#ifndef SILC_THREAD_H_
#define SILC_THREAD_H_

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef void* (*silc_thread_entry)(void* thread_arg);

typedef struct silc_thread_s
{
	pthread_t thread;
}silc_thread,* silc_thread_id;;

/**
 * Create a thread, which requires cleanup.
 * silc_thread_wait_destroy can be used to clean the thread up.
 * @param entry
 * @param arg
 * @return returns the thread id on success, the id can be then passed to silc_thread_wait_destroy. \n On failure, returns NULL
 *
 */
static inline silc_thread_id silc_thread_create(silc_thread_entry entry, void* arg)
{
	silc_thread_id ret_id = (silc_thread_id)malloc(sizeof(silc_thread));
	if(ret_id==NULL)
		return NULL;
	if(0!=pthread_create(&(ret_id->thread), NULL, entry, arg))
	{
		free(ret_id);
		return NULL;
	}
	return ret_id;
}

/**
 * Create a free running thread, which doesn't require cleanup
 *
 * @param entry
 * @param arg
 * @return 0 on sucess, -1 on failure.
 */
static inline int silc_thread_create_detached(silc_thread_entry entry, void* arg)
{
	pthread_t thread_id;
	if(0!=pthread_create(&thread_id, NULL, entry, arg))
	{
		return -1;
	}
	if(0!=pthread_detach(thread_id))
	{
		SILC_ERR("failed to detach thread");
	}

	return 0;
}

/**
 * Wait a thread to quit, and cleanup all resources.
 * @param p_thread
 */
static inline void silc_thread_wait_destroy(silc_thread_id p_thread)
{
	pthread_join(p_thread->thread, NULL);

	free(p_thread);
}

#ifdef __cplusplus
}
#endif

#endif /* SILC_THREAD_H_ */
