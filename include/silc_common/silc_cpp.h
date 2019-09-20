/*
 * silc_cpp.h
 *
 *  Created on: Nov 22, 2010
 *      Author: jeff_zheng
 *   Copyright: NetPerform Technology
 */

#ifndef SILC_CPP_H_
#define SILC_CPP_H_

#include <string>
#include "silc_common.h"
#include <iostream>
#include <iomanip>

#define SILC_LOG_CPP_TM()		"["<<silc_logtmstr<<"]"<<"[TSC:"<<std::dec<<std::setw(10)<<silc_tsc32_get()<<"]"

#define SILC_LOG_CPP_END_TASK()	 std::cerr.flush();} } while(0);

#if 1
#define SILC_CPP_DEBUG(...) SILC_LOG_PRE_TASK(SILC_LOG_LEVEL_DEBUG); std::cerr<<"[LOG]"<<SILC_LOG_CPP_TM()<< __VA_ARGS__ << "\n";SILC_LOG_CPP_END_TASK();
#define SILC_CPP_TRACE(...) SILC_LOG_PRE_TASK(SILC_LOG_LEVEL_INFO); std::cerr<<"[LOG]"<<SILC_LOG_CPP_TM()<< __VA_ARGS__ << "\n"; SILC_LOG_CPP_END_TASK();
#else
#define SILC_CPP_TRACE(...)
#endif
#define SILC_CPP_LOG(...) SILC_LOG_PRE_TASK(SILC_LOG_LEVEL_NOTICE); std::cerr<<"[LOG]"<<SILC_LOG_CPP_TM()<< __VA_ARGS__ << "\n"; SILC_LOG_CPP_END_TASK();
#define SILC_CPP_ERR(...) SILC_LOG_PRE_TASK(SILC_LOG_LEVEL_ERR); std::cerr<<"[ERR]"<<SILC_LOG_CPP_TM()<< __VA_ARGS__ << "\n"; SILC_LOG_CPP_END_TASK();

class silc_lockable
{
private:
	silc_mutex* _lock;

protected:
	inline silc_lockable() { _lock = silc_mutex_create(); if(_lock==NULL) { throw std::bad_alloc();} };
	void lock() { silc_mutex_lock(_lock); };
	void unlock() { silc_mutex_unlock(_lock); };

};

#define silc_singleton_decl(classname)		\
private:		\
	static pthread_once_t _once_ctl;		\
	static classname* _p_instance;			\
	static void _create_instance();			\
public:	\
	static classname& get_instance();		\


#define silc_singleton_impl(classname)		\
pthread_once_t classname::_once_ctl = PTHREAD_ONCE_INIT; \
classname* classname::_p_instance = NULL;	\
void classname::_create_instance(){	classname::_p_instance = new classname();} \
classname& classname::get_instance(){ \
	pthread_once(&classname::_once_ctl, classname::_create_instance); \
	return *classname::_p_instance; \
}


#endif /* SILC_CPP_H_ */
