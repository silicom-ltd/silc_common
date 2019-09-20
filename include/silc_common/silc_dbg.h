/*
 * silc_dbg.h
 *
 *  Created on: Mar 18, 2011
 *      Author: jeff_zheng
 *   Copyright: NetPerform Technology
 */

#ifndef SILC_DBG_H_
#define SILC_DBG_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "silc_common.h"
#include <inttypes.h>
#define silc_hex_prn(fmt, ...) \
	do {\
	plen = snprintf(buf+off, left, fmt, ## __VA_ARGS__); \
	off += plen; \
	if(plen>=left) \
	{ \
		buf[buf_size-1] = 0; \
		return; \
	} \
	left -= plen; \
	}while(0)
#define silc_hex_prn_asc(value)	\
	(isprint(value)? value : '_')

static inline void silc_hex_dump_str(void* to_dump, uint32_t size, char* buf, uint32_t buf_size)
{
	uint8_t* src = (uint8_t*)to_dump;
	uintptr_t loop, loop_save;
	uintptr_t base = (uintptr_t)src;
	uintptr_t base_rnd = base & 0xFFFFFFFFFFFFFFF0ULL;
	uint32_t off = 0;
	uint32_t left = buf_size-1;
	uintptr_t plen;

	silc_hex_prn("0x%"PRIxPTR":", base_rnd);
	for(loop = base_rnd; loop<base;loop++)
		silc_hex_prn("   ");

	uintptr_t loop_max = 16 - (base - base_rnd);
	if(loop_max > size)	loop_max = size;
	for(loop = 0; loop<loop_max; loop++)
		silc_hex_prn(" %02x", src[loop]);
	silc_hex_prn(" |");
	for(loop = 0; loop<loop_max; loop++)
		silc_hex_prn("%c", silc_hex_prn_asc(src[loop]));
	silc_hex_prn("\n");

	for(; (loop+16) < size; loop+=16)
	{
		base_rnd += 16;
		silc_hex_prn("0x%"PRIxPTR": %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x |"
				"%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
				base_rnd,
				src[loop], src[loop+1], src[loop+2], src[loop+3],
				src[loop+4], src[loop+5], src[loop+7], src[loop+6],
				src[loop+8], src[loop+9], src[loop+10], src[loop+11],
				src[loop+12], src[loop+13], src[loop+14], src[loop+15],
				silc_hex_prn_asc(src[loop]), silc_hex_prn_asc(src[loop+1]), silc_hex_prn_asc(src[loop+2]), silc_hex_prn_asc(src[loop+3]),
				silc_hex_prn_asc(src[loop+4]), silc_hex_prn_asc(src[loop+5]), silc_hex_prn_asc(src[loop+7]), silc_hex_prn_asc(src[loop+6]),
				silc_hex_prn_asc(src[loop+8]), silc_hex_prn_asc(src[loop+9]), silc_hex_prn_asc(src[loop+10]), silc_hex_prn_asc(src[loop+11]),
				silc_hex_prn_asc(src[loop+12]), silc_hex_prn_asc(src[loop+13]), silc_hex_prn_asc(src[loop+14]), silc_hex_prn_asc(src[loop+15]));
	}
	if(loop != size)
	{
		base_rnd += 16;
		silc_hex_prn("0x%"PRIxPTR":", base_rnd);
		loop_max = loop+16;
		loop_save = loop;
		for(; loop<size; loop++)
			silc_hex_prn(" %02x", src[loop]);
		for(; loop<loop_max; loop++)
			silc_hex_prn("   ");
		silc_hex_prn(" |");
		for(loop = loop_save; loop<size; loop++)
			silc_hex_prn("%c", silc_hex_prn_asc(src[loop]));

		silc_hex_prn("\n");
	}

}


#ifdef __cplusplus
}
#endif

#endif /* SILC_DBG_H_ */
