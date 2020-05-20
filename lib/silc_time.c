/*
 * silc_time.c
 *
 *  Created on: Nov 29, 2010
 *      Author: jeff_zheng
 *   Copyright: NetPerform Technology
 */
#include "silc_common.h"

void silc_time_sleep(uint32_t sec, uint32_t nanosec)
{
	struct timespec ts;
	int err;
	ts.tv_sec = sec;
	ts.tv_nsec = nanosec;
	err = nanosleep(&ts,NULL);
	if(err)
	{
		SILC_TRACE("silc_time_sleep returned earlier %s",strerror(err));
	}
}


static silc_bool g_silc_time_loop_enable = silc_false;
uint64_t g_silc_time_ns = 0;
uint64_t g_silc_time_us = 0;
uint64_t g_silc_time_ms = 0;
uint64_t g_silc_time_sec = 0;

uint64_t g_silc_time_sysup_ns = 0;
uint64_t g_silc_time_sysup_us = 0;
uint64_t g_silc_time_sysup_ms = 0;
uint64_t g_silc_time_sysup_sec = 0;
silc_sem g_silc_time_sleep_sem;
pthread_t g_silc_time_thread;

void silc_time_update()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	g_silc_time_sec = tv.tv_sec;
	g_silc_time_us = g_silc_time_sec* 1000000ULL + tv.tv_usec;
	g_silc_time_ns = g_silc_time_us* 1000;
	g_silc_time_ms = g_silc_time_us/ 1000;
}



void* silc_time_loop(void* arg)
{
	while(g_silc_time_loop_enable)
	{
		silc_time_update();
		//sleep 500 us
		silc_sem_wait(&g_silc_time_sleep_sem, 500000);
	}
	return NULL;
}


void silc_time_lib_init_once(void)
{
	silc_time_update();
//	pthread_attr_t attr;
//	pthread_attr_init(&attr);
//	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	//first update
	silc_time_update();

	g_silc_time_sysup_ns = g_silc_time_ns;
	g_silc_time_sysup_us = g_silc_time_us;
	g_silc_time_sysup_ms = g_silc_time_ms;
	g_silc_time_sysup_sec = g_silc_time_sec;

	silc_sem_init(&g_silc_time_sleep_sem);

//	if(0!=pthread_create(&t, &attr, silc_time_loop, NULL))
//	{
//		return;
//	}
//	return;
	g_silc_time_loop_enable = silc_true;
	if(0!=pthread_create(&g_silc_time_thread, NULL, silc_time_loop, NULL))
	{
		g_silc_time_loop_enable = silc_false;
	}
}

#if defined(MIPS64_NLM_LINUX) || defined(POWERPC_FSL_LINUX) || defined(ARM_LINUX) || defined(AARCH64_MARVELL_LINUX)
void silc_tsc32_read(uint32_t* tsc)
{
	*tsc = 0;
}

void silc_tsc64_read(uint64_t* tsc64)
{
	*tsc64 = 0;
}
#else
void silc_tsc32_read(uint32_t* tsc)
{
__asm__ __volatile__("rdtsc" : "=a" (*tsc) : : "edx");
}

void silc_tsc64_read(uint64_t* tsc64)
{
     unsigned int __a,__d;
     __asm__ __volatile__("rdtsc" : "=a" (__a), "=d" (__d));
     *tsc64 = ((uint64_t)__a) | (((uint64_t)__d)<<32);
}
#endif

uint32_t silc_tsc32_get(void)
{
    uint32_t tsc;
    silc_tsc32_read(&tsc);
    return tsc;
}

pthread_once_t g_silc_time_lib_once = PTHREAD_ONCE_INIT;

void silc_time_lib_init(void)
{
	pthread_once(&g_silc_time_lib_once, silc_time_lib_init_once);
}

void silc_time_lib_deinit(void)
{
	if(g_silc_time_loop_enable)
	{
		g_silc_time_loop_enable = silc_false;
		silc_sem_give(&g_silc_time_sleep_sem);
		pthread_join(g_silc_time_thread, NULL);
	}
}
