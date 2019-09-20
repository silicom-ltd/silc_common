/*
 * silc_id_gen.h
 *
 *  Created on: Oct 21, 2011
 *      Author: jeff.zheng
 */

#ifndef SILC_ID_GEN_H_
#define SILC_ID_GEN_H_

#include "silc_common.h"

class silc_id_gen {
private:
//	pthread_once_t 	_global_init_once;
	silc_mutex*		_id_lock;
	uint32_t		_global_id;


public:
	void 		lock();
	void 		unlock();
	uint32_t	get_new_id();
	silc_id_gen();
	~silc_id_gen();
	void set_max_id(uint32_t id){if(id>=_global_id) _global_id = id+1;};
};

#endif /* SILC_ID_GEN_H_ */
