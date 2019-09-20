/*
 * silc_init.c
 *
 *  Created on: Feb 5, 2011
 *      Author: jeff_zheng
 */
#include "silc_common.h"

volatile int g_silc_common_init_return = -1;
void silc_common_init_inner()
{
	silc_time_lib_init();
	g_silc_common_init_return = 0;
	return;
}

pthread_once_t g_silc_common_init_once = PTHREAD_ONCE_INIT;
int silc_common_init(void)
{
	pthread_once(&g_silc_common_init_once, silc_common_init_inner);
	return g_silc_common_init_return;
}

uint64_t g_silc_mcache_global_mem_limit = 0x10000000000ULL;  //1TB, big enough, we probably will never have this much memory
uint64_t g_silc_mcache_global_mem_current = 0;
#ifdef POWERPC_FSL_LINUX
pthread_mutex_t g_silc_mcache_global_mem_lock = PTHREAD_MUTEX_INITIALIZER;
#endif
