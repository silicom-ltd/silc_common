/*
 * silc_common.h
 *
 *  Created on: Nov 22, 2010
 *      Author: jeff_zheng
 *   Copyright: NetPerform Technology
 */

#ifndef SILC_COMMON_H_
#define SILC_COMMON_H_

#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <pthread.h>
#include <string.h>
/*file access*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/un.h>

/*basic types*/
#include "silc_common/silc_types.h"
#include "silc_common/silc_math.h"
#include "silc_common/silc_sem.h"
/*basic services*/
#include "silc_common/silc_logging.h"
#include "silc_common/silc_time.h"
/*basic data structures*/

#include "silc_common/silc_str.h"
#include "silc_common/silc_list.h"
#include "silc_common/silc_mem.h"
#include "silc_common/silc_array.h"
#include "silc_common/silc_htbl.h"
#include "silc_common/silc_ring_buffer.h"

/*basic services*/
#include "silc_common/silc_file.h"

/*other complex servcies*/
#include "silc_common/silc_socket.h"

#include "silc_common/silc_stats.h"

#include "silc_common/silc_gpio.h"
#ifndef __cplusplus
#include "silc_common/silc_dbg.h"
#endif


#include "silc_common/silc_cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int silc_common_init(void);


#ifdef __cplusplus
}
#endif
#endif /* SILC_COMMON_H_ */
