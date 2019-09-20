/*
 * silc_cpu.h
 *
 *  Created on: Jun 23, 2011
 *      Author: jeff_zheng
 *   Copyright: NetPerform Technology
 */

#ifndef SILC_CPU_H_
#define SILC_CPU_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _GNU_SOURCE
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/types.h>
#include <sched.h>
static inline int silc_cpu_bind_cpu(int cpu_no)
{
	cpu_set_t	cpd_cpu_set_val;
	pid_t		self_tid;
	if(cpu_no>CPU_SETSIZE)
		return -1;

	self_tid = syscall(__NR_gettid);
	sched_getaffinity(self_tid, sizeof(cpd_cpu_set_val),  &cpd_cpu_set_val);

	CPU_ZERO(&cpd_cpu_set_val);
	CPU_SET(cpu_no,&cpd_cpu_set_val);

	if(0!=sched_setaffinity(self_tid , sizeof(cpd_cpu_set_val), &cpd_cpu_set_val))
	{
		SILC_ERR("set affinity failed for cpu_no: %d, cpu_no: %d, %s",
				self_tid, cpu_no,  strerror(errno));
		return -1;
	}
	sched_getaffinity(self_tid, sizeof(cpd_cpu_set_val),  &cpd_cpu_set_val);
	SILC_LOG("set affinity for cpu_no: %d, cpu_no: %d, new affinity is ",self_tid, cpu_no);
	return 0;
}


//set priority, 0 is the highest,
static inline void silc_cpu_sched_set_realtime(int new_pri)
{
	pid_t	self_tid;
	int		pri_max;
	struct	sched_param pri;


	self_tid = syscall(__NR_gettid);
	pri_max = sched_get_priority_max(SCHED_RR);
	if(pri_max<0)
	{
		SILC_ERR("Failed to get priority max from scheduler");
		return;
	}
	pri.sched_priority = pri_max - new_pri;
	if(sched_setscheduler(self_tid, SCHED_RR, &pri)<0)
	{
		SILC_ERR("Failed to set priority for thread, %u", pri_max);
	}

	return;
}

static inline void silc_cpu_sched_set_low(void)
{
	pid_t	self_tid;
	int pri_max, pri_min;
	struct sched_param pri;

	self_tid = syscall(__NR_gettid);
	pri_max = sched_get_priority_max(SCHED_OTHER);
	if(pri_max<0)
	{
		SILC_ERR("Failed to get priority max from scheduler");
		return;
	}
	pri_min = sched_get_priority_max(SCHED_OTHER);
	if(pri_min<0)
	{
		SILC_ERR("Failed to get priority max from scheduler");
		return;
	}
	SILC_LOG("RR min:%u, max%u", pri_min, pri_max);
	pri.sched_priority = pri_min;
	if(sched_setscheduler(self_tid, SCHED_OTHER, &pri)<0)
	{
		SILC_ERR("Failed to set priority for thread, %u", pri_max);
	}

	return;
}

#endif


#ifdef __cplusplus
}
#endif

#endif /* SILC_CPU_H_ */
