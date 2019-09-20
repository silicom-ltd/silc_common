/*
 * silc_sem.h
 *
 *  Created on: Dec 8, 2010
 *      Author: jeff_zheng
 *   Copyright: NetPerform Technology
 */

#ifndef SILC_SEM_H_
#define SILC_SEM_H_

#include "pthread.h"
#include "semaphore.h"
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct silc_mutex_s
{
	pthread_mutex_t	m;
}silc_mutex;


static inline silc_mutex* silc_mutex_create(void)
{
	silc_mutex* p_m = (silc_mutex*)malloc(sizeof(silc_mutex));
	pthread_mutexattr_t attr;
	if(0!=pthread_mutexattr_init(&attr))
	{
		free(p_m);
		return NULL;
	}
	if(0!=pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP))
	{
		free(p_m);
		return NULL;
	}
	if(0!=pthread_mutex_init(&p_m->m, &attr))
	{
		free(p_m);
		return NULL;
	}
	pthread_mutexattr_destroy(&attr);
	return p_m;
}

static inline int silc_mutex_destroy(silc_mutex* p_m)
{
	pthread_mutex_destroy(&p_m->m);
	free(p_m);

	return 0;
}

static inline int silc_mutex_lock(silc_mutex* p_m)
{
	int r = pthread_mutex_lock(&p_m->m);
	if(r!=0)
	{
		fprintf(stderr,"Failed to lock mutext, %s\n", strerror(errno));
	}
	return r;
}
static inline int silc_mutex_unlock(silc_mutex* p_m)
{
	int r = pthread_mutex_unlock(&p_m->m);
	if(r!=0)
	{
		fprintf(stderr,"Failed to unlock mutext, %s\n", strerror(errno));
	}
	return r;
}

typedef sem_t silc_sem;

static inline int silc_sem_init(silc_sem* p_sem)
{
	return sem_init(p_sem, 0 , 0);
}

/**
 * Wait for a semaphore with timeout in us.
 * if timeout is below 0, wait forever until sem is not empty
 * if timeout is 0, return immediately, 0 on success -1 on error
 * if timeout is above 0, wait for the specified timeout is sem is empty
 * @param p_sem
 * @param time_us
 * @return
 */
static inline int silc_sem_wait(silc_sem* p_sem, int time_us)
{
	if(time_us <0)
		return sem_wait(p_sem);
	else if(time_us == 0)
		return sem_trywait(p_sem);
	else
	{
		struct timespec ts;
		struct timeval tv;
		uint32_t sec, usec;
		gettimeofday(&tv, NULL);
		sec = time_us/1000000;
		usec = time_us%1000000;
		ts.tv_sec = tv.tv_sec + sec + (tv.tv_usec+usec)/1000000;
		ts.tv_nsec = (tv.tv_usec+usec)%1000000;
		ts.tv_nsec *= 1000;
		return sem_timedwait(p_sem, &ts);
	}
}

static inline int silc_sem_give(silc_sem* p_sem)
{
	return sem_post(p_sem);
}

#ifdef __cplusplus
}
#endif

#endif /* SILC_SEM_H_ */
