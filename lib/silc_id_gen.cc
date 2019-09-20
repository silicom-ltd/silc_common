/*
 * silc_id_gen.cpp
 *
 *  Created on: Oct 21, 2010
 *      Author: jeff.zheng
 */

#include "silc_common/silc_id_gen.h"


silc_id_gen::silc_id_gen() {
	this->_id_lock = silc_mutex_create();
	_global_id = 0;
//	cl_monitor_point::_global_init_once;
}

silc_id_gen::~silc_id_gen() {
	// TODO Auto-generated destructor stub
	silc_mutex_destroy(this->_id_lock);
}

void silc_id_gen::lock()
{
	silc_mutex_lock(this->_id_lock);
}
void silc_id_gen::unlock()
{
	silc_mutex_unlock(this->_id_lock);
}
uint32_t silc_id_gen::get_new_id()
{
	return _global_id++;
}

