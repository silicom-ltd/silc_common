/*
 * silc_logging.h
 *
 *  Created on: Nov 22, 2010
 *      Author: jeff_zheng
 *   Copyright: NetPerform Technology
 */

#ifndef SILC_LOGGING_H_
#define SILC_LOGGING_H_

#ifdef __cplusplus
extern "C" {
#endif


typedef enum silc_log_level_e
{
	SILC_LOG_LEVEL_CRIT		= 0,
	SILC_LOG_LEVEL_ERR		= 1,
	SILC_LOG_LEVEL_NOTICE		= 2,
	SILC_LOG_LEVEL_INFO		= 3,
	SILC_LOG_LEVEL_DEBUG		= 4,
}silc_log_level_t;

extern int g_silc_log_level;

static inline void silc_log_level_set(silc_log_level_t level) { g_silc_log_level = level; };
static inline void silc_log_level_incr(void) { g_silc_log_level = (int)g_silc_log_level + 1; };

#define SILC_LOG_SYSLOG
#ifdef SILC_LOG_SMRLOG
#include "logging.h"

#define SILC_TRACE(fmt, ...)		lc_log_basic(LOG_DEBUG, fmt, ## __VA_ARGS__);

#define SILC_INFO(fmt, ...)		lc_log_basic(LOG_INFO, fmt, ## __VA_ARGS__);

#define SILC_LOG(fmt, ...)		lc_log_basic(LOG_NOTICE, fmt, ## __VA_ARGS__);

#define SILC_ERR(fmt, ...)		lc_log_basic(LOG_ERR, fmt, ## __VA_ARGS__);

#define SILC_CRIT(fmt, ...)		lc_log_basic(LOG_CRIT, fmt, ## __VA_ARGS__);

#elif defined(SILC_LOG_SYSLOG)
#include <syslog.h>

#define SILC_TRACE(fmt, ...)		syslog(LOG_DEBUG, fmt, ## __VA_ARGS__);

#define SILC_INFO(fmt, ...)		syslog(LOG_INFO, fmt, ## __VA_ARGS__);

#define SILC_LOG(fmt, ...)		syslog(LOG_NOTICE, fmt, ## __VA_ARGS__);

#define SILC_ERR(fmt, ...)		syslog(LOG_ERR, fmt, ## __VA_ARGS__);

#define SILC_CRIT(fmt, ...)		syslog(LOG_CRIT, fmt, ## __VA_ARGS__);

#else

#define SILC_LOG_PRE_TASK(lvl)	do { if(g_silc_log_level>=(lvl)){ \
							char silc_logtmstr[32];\
							silc_str_from_time(silc_time_get_time(), silc_logtmstr, sizeof(silc_logtmstr));

#define SILC_LOG_TM_FMT		"[%s][TSC:%10u]"
#define SILC_LOG_TM_PARAM()	silc_logtmstr,silc_tsc32_get()
#define SILC_LOG_END_TASK()	} } while(0);


#if 1
#define SILC_INFO(fmt, ...)
#else

#define SILC_INFO(fmt, ...)		SILC_LOG_PRE_TASK(SILC_LOG_LEVEL_INFO); \
								fprintf(stderr,"[INF]"SILC_LOG_TM_FMT fmt"\n", SILC_LOG_TM_PARAM(), ## __VA_ARGS__);\
								SILC_LOG_END_TASK();
#endif

#define SILC_LOG(fmt, ...)		SILC_LOG_PRE_TASK(SILC_LOG_LEVEL_NOTICE); \
								fprintf(stderr,"[LOG]"SILC_LOG_TM_FMT fmt"\n", SILC_LOG_TM_PARAM(), ## __VA_ARGS__);\
								SILC_LOG_END_TASK();
#define SILC_ERR(fmt, ...)		SILC_LOG_PRE_TASK(SILC_LOG_LEVEL_ERR); \
								fprintf(stderr,"[ERR]"SILC_LOG_TM_FMT fmt"\n", SILC_LOG_TM_PARAM(), ## __VA_ARGS__);\
								SILC_LOG_END_TASK();
#define SILC_CRIT(fmt, ...)		SILC_LOG_PRE_TASK(SILC_LOG_LEVEL_CRIT); \
								fprintf(stderr,"[CRI]"SILC_LOG_TM_FMT fmt"\n", SILC_LOG_TM_PARAM(), ## __VA_ARGS__);\
								SILC_LOG_END_TASK();
#if 1
#  define SILC_TRACE(fmt, ...)	SILC_LOG_PRE_TASK(SILC_LOG_LEVEL_INFO); fprintf(stderr,"[TRC]"SILC_LOG_TM_FMT fmt"\n", SILC_LOG_TM_PARAM(), ## __VA_ARGS__);SILC_LOG_END_TASK();
#  define SILC_DEBUG(fmt, ...)	SILC_LOG_PRE_TASK(SILC_LOG_LEVEL_DEBUG); fprintf(stderr,"[DBG]"SILC_LOG_TM_FMT fmt"\n", SILC_LOG_TM_PARAM(), ## __VA_ARGS__);SILC_LOG_END_TASK();
#else
#  define SILC_TRACE(fmt, ...)
#  define SILC_DEBUG(fmt, ...)
#endif


#endif



#ifdef __cplusplus
}
#endif

#endif /* SILC_LOGGING_H_ */
