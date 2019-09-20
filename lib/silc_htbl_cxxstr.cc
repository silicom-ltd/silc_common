/*
 * silc_htbl_str.cc
 *
 *  Created on: Dec 4, 2010
 *      Author: jeff_zheng
 */
#include "silc_common.h"

using namespace std;
uint32_t silc_htbl_default_cxxstr_hash(string* p_str)
{
	uint64_t r = silc_htbl_jenkins_hash((unsigned char*)(p_str)->c_str(), (p_str)->length());
	return r;
}

int silc_htbl_default_cxxstr_comp(const string* p_str1, const string* p_str2)
{
	return strcmp((p_str1)->c_str(), (p_str2)->c_str());
}





