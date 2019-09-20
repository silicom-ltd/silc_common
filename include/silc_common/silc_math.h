/*
 * silc_math.h
 *
 *  Created on: Apr 16, 2011
 *      Author: jeff_zheng
 *   Copyright: NetPerform Technology
 */

#ifndef SILC_MATH_H_
#define SILC_MATH_H_

#ifdef __cplusplus
extern "C" {
#endif

#define silc_math_declare(dtype, typename) \
static inline dtype silc_min_##typename(dtype a, dtype b) \
{ \
	return a>b?b:a; \
} \
 \
static inline dtype silc_max_##typename(dtype a, dtype b) \
{ \
	return a>b?a:b; \
}

silc_math_declare(uint32_t, u32);
silc_math_declare(uint64_t, u64);
silc_math_declare(int32_t, i32);
silc_math_declare(int64_t, i64);

#ifdef __cplusplus
}
#endif

#endif /* SILC_MATH_H_ */
