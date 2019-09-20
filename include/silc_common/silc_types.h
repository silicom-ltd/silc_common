/*
 * silc_types.h
 *
 *  Created on: Nov 22, 2010
 *      Author: jeff_zheng
 *   Copyright: NetPerform Technology
 */

#ifndef SILC_TYPES_H_
#define SILC_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif


typedef enum silc_bool_e{
	silc_false = 0,
	silc_true = 1
}silc_bool;

#define SILC_SIZE_MB	(1ULL<<20)

#ifdef __cplusplus
}
#endif

#endif /* SILC_TYPES_H_ */
