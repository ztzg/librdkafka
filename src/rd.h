/*
 * librd - Rapid Development C library
 *
 * Copyright (c) 2012, Magnus Edenhill
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef _RD_H_
#define _RD_H_

#ifndef _MSC_VER
#ifndef _GNU_SOURCE
#define _GNU_SOURCE  /* for strndup() */
#endif

#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE /* for strlcpy, pthread_setname_np, etc */
#endif

#define __need_IOV_MAX
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L  /* for timespec on solaris */
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <limits.h>

#ifdef _MSC_VER
/* Visual Studio */
#include "win32_config.h"
#else
/* POSIX / UNIX based systems */
#include "../config.h" /* mklove output */
#endif

#ifdef _MSC_VER
/* Win32/Visual Studio */
#include "rdwin32.h"

#else
/* POSIX / UNIX based systems */
#include "rdposix.h"
#endif

#include "rdtypes.h"


/* Debug assert, only enabled with --enable-devel */
#if ENABLE_DEVEL == 1
#define rd_dassert(cond) rd_assert(cond)
#else
#define rd_dassert(cond)  do {} while (0)
#endif


/** Assert if reached */
#define RD_NOTREACHED() rd_assert(!*"/* NOTREACHED */ violated")



/**
* Allocator wrappers.
* We serve under the premise that if a (small) memory
* allocation fails all hope is lost and the application
* will fail anyway, so no need to handle it handsomely.
*/
static RD_INLINE RD_UNUSED void *rd_calloc(size_t num, size_t sz) {
	void *p = calloc(num, sz);
	rd_assert(p);
	return p;
}

static RD_INLINE RD_UNUSED void *rd_malloc(size_t sz) {
	void *p = malloc(sz);
	rd_assert(p);
	return p;
}

static RD_INLINE RD_UNUSED void *rd_realloc(void *ptr, size_t sz) {
	void *p = realloc(ptr, sz);
	rd_assert(p);
	return p;
}

static RD_INLINE RD_UNUSED void rd_free(void *ptr) {
	free(ptr);
}

static RD_INLINE RD_UNUSED char *rd_strdup(const char *s) {
#ifndef _MSC_VER
	char *n = strdup(s);
#else
	char *n = _strdup(s);
#endif
	rd_assert(n);
	return n;
}

static RD_INLINE RD_UNUSED char *rd_strndup(const char *s, size_t len) {
#if HAVE_STRNDUP
	char *n = strndup(s, len);
	rd_assert(n);
#else
	char *n = malloc(len + 1);
	rd_assert(n);
	memcpy(n, s, len);
	n[len] = '\0';
#endif
	return n;
}



/*
 * Portability
 */

#ifdef strndupa
#define rd_strndupa(DESTPTR,PTR,LEN)  (*(DESTPTR) = strndupa(PTR,LEN))
#else
#define rd_strndupa(DESTPTR,PTR,LEN) do {                               \
                const char *_src = (PTR);                               \
                size_t _srclen = (LEN);                                 \
                char *_dst = rd_alloca(_srclen + 1);                    \
                memcpy(_dst, _src, _srclen);                            \
                _dst[_srclen] = '\0';                                   \
                *(DESTPTR) = _dst;                                      \
        } while (0)
#endif

#ifdef strdupa
#define rd_strdupa(DESTPTR,PTR)  (*(DESTPTR) = strdupa(PTR))
#else
#define rd_strdupa(DESTPTR,PTR) do {                                    \
                const char *_src1 = (PTR);                              \
                size_t _srclen1 = strlen(_src1);                        \
                rd_strndupa(DESTPTR, _src1, _srclen1);                  \
        } while (0)
#endif

#ifndef IOV_MAX
#ifdef __APPLE__
/* Some versions of MacOSX dont have IOV_MAX */
#define IOV_MAX 1024
#elif defined(_MSC_VER) || defined(__GNU__)
/* There is no IOV_MAX on MSVC or GNU but it is used internally in librdkafka */
#define IOV_MAX 1024
#else
#error "IOV_MAX not defined"
#endif
#endif


/* Round/align X upwards to STRIDE, which must be power of 2. */
#define RD_ROUNDUP(X,STRIDE) (((X) + ((STRIDE) - 1)) & ~(STRIDE-1))

#define RD_ARRAY_SIZE(A)          (sizeof((A)) / sizeof(*(A)))
#define RD_ARRAYSIZE(A)           RD_ARRAY_SIZE(A)
#define RD_SIZEOF(TYPE,MEMBER)    sizeof(((TYPE *)NULL)->MEMBER)
#define RD_OFFSETOF(TYPE,MEMBER)  ((size_t) &(((TYPE *)NULL)->MEMBER))

/**
 * Returns the 'I'th array element from static sized array 'A'
 * or NULL if 'I' is out of range.
 * var-args is an optional prefix to provide the correct return type.
 */
#define RD_ARRAY_ELEM(A,I,...)				\
	((unsigned int)(I) < RD_ARRAY_SIZE(A) ? __VA_ARGS__ (A)[(I)] : NULL)


#define RD_STRINGIFY(X)  # X



#define RD_MIN(a,b) ((a) < (b) ? (a) : (b))
#define RD_MAX(a,b) ((a) > (b) ? (a) : (b))


/**
 * Cap an integer (of any type) to reside within the defined limit.
 */
#define RD_INT_CAP(val,low,hi) \
	((val) < (low) ? low : ((val) > (hi) ? (hi) : (val)))



/**
 * Allocate 'size' bytes, copy 'src', return pointer to new memory.
 *
 * Use rd_free() to free the returned pointer.
*/
static RD_INLINE RD_UNUSED void *rd_memdup (const void *src, size_t size) {
	void *dst = rd_malloc(size);
	memcpy(dst, src, size);
	return dst;
}

/**
 * @brief Memset &OBJ to 0, does automatic sizeof(OBJ).
 */
#define RD_MEMZERO(OBJ) memset(&(OBJ), 0, sizeof(OBJ))

#endif /* _RD_H_ */
