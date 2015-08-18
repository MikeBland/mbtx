/* ISO C9x  7.18  Integer types <stdint.h>
 * Based on ISO/IEC SC22/WG14 9899 Committee draft (SC22 N2794)
 *
 *  THIS SOFTWARE IS NOT COPYRIGHTED
 *
 *  Contributor: ATMEL Norway AS
 *  Adapted to IAR C Compiler for Atmel AVR 8-bit RISC MCU.
 *
 *  This source code is offered for use in the public domain. You may
 *  use, modify or distribute it freely.
 *
 *  This code is distributed in the hope that it will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAIMED. This includes but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 *  Date: 2005-12-20
 */


#ifndef _STDINT_H
#define _STDINT_H
#include <stddef.h>

/* 7.18.1.1  Exact-width integer types */
typedef signed char        int8_t;
typedef unsigned char      uint8_t;
typedef signed short       int16_t;
typedef unsigned short     uint16_t;
typedef signed long        int32_t;
typedef unsigned long      uint32_t;
typedef signed long long   int64_t;
typedef unsigned long long uint64_t;

/* 7.18.1.2  Minimum-width integer types */
typedef signed char        int_least8_t;
typedef unsigned char      uint_least8_t;
typedef signed short       int_least16_t;
typedef unsigned short     uint_least16_t;
typedef signed long        int_least32_t;
typedef unsigned long      uint_least32_t;
typedef signed long long   int_least64_t;
typedef unsigned long long uint_least64_t;

/*  7.18.1.3  Fastest minimum-width integer types
 *  Not actually guaranteed to be fastest for all purposes
 *  Here we use the exact-width types for 8 and 16-bit ints.
 */
typedef signed char        int_fast8_t;
typedef unsigned char      uint_fast8_t;
typedef signed short       int_fast16_t;
typedef unsigned short     uint_fast16_t;
typedef signed long        int_fast32_t;
typedef unsigned long      uint_fast32_t;
typedef signed long long   int_fast64_t;
typedef unsigned long long uint_fast64_t;

/* 7.18.1.4  Integer types capable of holding object pointers */
// Already defined by IAR, and differs between specific devices.
//typedef signed int         intptr_t;
//typedef unsigned           uintptr_t;

/* 7.18.1.5  Greatest-width integer types */
typedef signed long long   intmax_t;
typedef unsigned long long uintmax_t;

/* 7.18.2  Limits of specified-width integer types */
#if !defined ( __cplusplus) || defined (__STDC_LIMIT_MACROS)

/* 7.18.2.1  Limits of exact-width integer types */
#define INT8_MIN (-128)
#define INT16_MIN (-32768)
#define INT32_MIN (-2147483647 - 1)
#define INT64_MIN  (-9223372036854775807LL - 1)

#define INT8_MAX 127
#define INT16_MAX 32767
#define INT32_MAX 2147483647
#define INT64_MAX 9223372036854775807LL

#define UINT8_MAX 0xff /* 255U */
#define UINT16_MAX 0xffff /* 65535U */
#define UINT32_MAX 0xffffffff  /* 4294967295U */
#define UINT64_MAX 0xffffffffffffffffULL /* 18446744073709551615ULL */

/* 7.18.2.2  Limits of minimum-width integer types */
#define INT_LEAST8_MIN INT8_MIN
#define INT_LEAST16_MIN INT16_MIN
#define INT_LEAST32_MIN INT32_MIN
#define INT_LEAST64_MIN INT64_MIN

#define INT_LEAST8_MAX INT8_MAX
#define INT_LEAST16_MAX INT16_MAX
#define INT_LEAST32_MAX INT32_MAX
#define INT_LEAST64_MAX INT64_MAX

#define UINT_LEAST8_MAX UINT8_MAX
#define UINT_LEAST16_MAX UINT16_MAX
#define UINT_LEAST32_MAX UINT32_MAX
#define UINT_LEAST64_MAX UINT64_MAX

/* 7.18.2.3  Limits of fastest minimum-width integer types */
#define INT_FAST8_MIN INT8_MIN
#define INT_FAST16_MIN INT16_MIN
#define INT_FAST32_MIN INT32_MIN
#define INT_FAST64_MIN INT64_MIN

#define INT_FAST8_MAX INT8_MAX
#define INT_FAST16_MAX INT16_MAX
#define INT_FAST32_MAX INT32_MAX
#define INT_FAST64_MAX INT64_MAX

#define UINT_FAST8_MAX UINT8_MAX
#define UINT_FAST16_MAX UINT16_MAX
#define UINT_FAST32_MAX UINT32_MAX
#define UINT_FAST64_MAX UINT64_MAX

/* 7.18.2.4  Limits of integer types capable of holding
    object pointers */
#define INTPTR_MIN INT32_MIN
#define INTPTR_MAX INT32_MAX
#define UINTPTR_MAX UINT32_MAX

/* 7.18.2.5  Limits of greatest-width integer types */
#define INTMAX_MIN INT64_MIN
#define INTMAX_MAX INT64_MAX
#define UINTMAX_MAX UINT64_MAX

/* 7.18.3  Limits of other integer types */
#define PTRDIFF_MAX ((ptrdiff_t)-1)
#define PTRDIFF_MIN (-PTRDIFF_MAX)

#define SIZE_MAX ((size_t)-1)

/* --- The following are also defined in IAR's wchar.h file ---
#ifndef WCHAR_MIN 
#define WCHAR_MIN 0
#define WCHAR_MAX ((wchar_t)-1)
#endif
*/

#endif /* !defined ( __cplusplus) || defined __STDC_LIMIT_MACROS */


/* 7.18.4  Macros for integer constants */
#if !defined ( __cplusplus) || defined (__STDC_CONSTANT_MACROS)

/* 7.18.4.1  Macros for minimum-width integer constants */
#define INT8_C(val) ((int8_t) + (val))
#define UINT8_C(val) ((uint8_t) + (val##U))
#define INT16_C(val) ((int16_t) + (val))
#define UINT16_C(val) ((uint16_t) + (val##U))

#define INT32_C(val) (val##L)
#define UINT32_C(val) (val##UL)
#define INT64_C(val) (val##LL)
#define UINT64_C(val) (val##ULL)

/* 7.18.4.2  Macros for greatest-width integer constants */
#define INTMAX_C(val)  INT64_C(val)
#define UINTMAX_C(val) UINT64_C(val)

#endif  /* !defined ( __cplusplus) || defined __STDC_CONSTANT_MACROS */

#endif

