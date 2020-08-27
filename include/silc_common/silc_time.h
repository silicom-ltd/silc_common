/*
 * silc_time.h
 *
 *  Created on: Nov 22, 2010
 *      Author: jeff_zheng
 *   Copyright: NetPerform Technology
 */

#ifndef SILC_TIME_H_
#define SILC_TIME_H_

#include <time.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define silc_dag_ts_to_ms(dag_ts)  (((dag_ts)>>32)*1000 + ((dag_ts)&UINT32_MAX)/(UINT32_MAX/1000))

static inline int silc_str_from_time(const uint64_t time_sec, char* result, size_t length)
{
	struct tm tm;
	gmtime_r((time_t*)&time_sec, &tm);

	result[0]=0;

	size_t written = strftime(result, length, "%F %T", &tm);
	if (written == 0)
		return -1;
	if (written >= length)
		return -1;
	return 0;
}

extern uint32_t silc_tsc32_get(void);

extern uint64_t g_silc_time_ns;
extern uint64_t g_silc_time_us;
extern uint64_t g_silc_time_ms;
extern uint64_t g_silc_time_sec;
extern uint64_t g_silc_time_fix_ms;

extern uint64_t g_silc_time_sysup_ns;
extern uint64_t g_silc_time_sysup_us;
extern uint64_t g_silc_time_sysup_ms;
extern uint64_t g_silc_time_sysup_sec;
static inline uint64_t silc_time_get_time(void)
{
	return g_silc_time_sec;
}

static inline uint64_t silc_time_get_ns(void)
{
	return g_silc_time_ns;
}
static inline uint64_t silc_time_get_us(void)
{
	return g_silc_time_us;
}
static inline uint64_t silc_time_get_ms(void)
{
	return g_silc_time_ms;
}
static inline uint64_t silc_time_get_fix_ms(void)
{
	return g_silc_time_fix_ms;
}


static inline uint64_t silc_time_get_sysup_ms()
{
	return g_silc_time_ms - g_silc_time_sysup_ms;
}

typedef struct silc_time_us_s
{
	union
	{
		struct
		{//little endian
			uint32_t us;
			uint32_t sec;
		}tv32;
		uint64_t tv64;
	}v;
}silc_time_us;

static inline void silc_time_get_time_us(silc_time_us* us)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    us->v.tv32.sec = tv.tv_sec;
    us->v.tv32.us  = tv.tv_usec;
}



extern void silc_time_sleep(uint32_t sec, uint32_t nanosec);

extern void silc_time_lib_init(void);

extern void silc_time_lib_deinit(void);


/*\@}*/


#ifdef __cplusplus
}
#endif


#endif /* SILC_TIME_H_ */
