/*
 * librdkafka - Apache Kafka C library
 *
 * Copyright (c) 2012-2013, Magnus Edenhill
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

#ifndef _RDKAFKA_INT_H_
#define _RDKAFKA_INT_H_

#ifndef _MSC_VER
#define _GNU_SOURCE  /* for strndup() */
#include <syslog.h>
#else
typedef int mode_t;
#endif
#include <fcntl.h>

#include "rdkafka.h"
#include "rd.h"

/* Forward declarations */
struct rd_kafka_s;
struct rd_kafka_broker_s;

#include "rdkafka_conf.h"
#include "rdkafka_transport.h"

/**
 * Kafka handle, internal representation of the application's rd_kafka_t.
 */

struct rd_kafka_s {
        rd_kafka_conf_t  rk_conf;
};

#include "rdkafka_broker.h"

/**
 * Debug contexts
 */
#define RD_KAFKA_DBG_GENERIC        0x1
#define RD_KAFKA_DBG_BROKER         0x2
#define RD_KAFKA_DBG_TOPIC          0x4
#define RD_KAFKA_DBG_METADATA       0x8
#define RD_KAFKA_DBG_FEATURE        0x10
#define RD_KAFKA_DBG_QUEUE          0x20
#define RD_KAFKA_DBG_MSG            0x40
#define RD_KAFKA_DBG_PROTOCOL       0x80
#define RD_KAFKA_DBG_CGRP           0x100
#define RD_KAFKA_DBG_SECURITY       0x200
#define RD_KAFKA_DBG_FETCH          0x400
#define RD_KAFKA_DBG_INTERCEPTOR    0x800
#define RD_KAFKA_DBG_PLUGIN         0x1000
#define RD_KAFKA_DBG_CONSUMER       0x2000
#define RD_KAFKA_DBG_ADMIN          0x4000
#define RD_KAFKA_DBG_EOS            0x8000
#define RD_KAFKA_DBG_MOCK           0x10000
#define RD_KAFKA_DBG_ALL            0xfffff
#define RD_KAFKA_DBG_NONE           0x0

void rd_kafka_log0(const rd_kafka_conf_t *conf,
                   const rd_kafka_t *rk, const char *extra, int level,
                   const char *fac, const char *fmt, ...) RD_FORMAT(printf,
                                                                    6, 7);

#define rd_kafka_log(rk,level,fac,...) \
        rd_kafka_log0(&rk->rk_conf, rk, NULL, level, fac, __VA_ARGS__)
#define rd_kafka_dbg(rk,ctx,fac,...) do {                               \
                if (unlikely((rk)->rk_conf.debug & (RD_KAFKA_DBG_ ## ctx))) \
                        rd_kafka_log0(&rk->rk_conf,rk,NULL,             \
                                      LOG_DEBUG,fac,__VA_ARGS__);       \
        } while (0)

/* dbg() not requiring an rk, just the conf object, for early logging */
#define rd_kafka_dbg0(conf,ctx,fac,...) do {                            \
                if (unlikely((conf)->debug & (RD_KAFKA_DBG_ ## ctx)))   \
                        rd_kafka_log0(conf,NULL,NULL,                   \
                                      LOG_DEBUG,fac,__VA_ARGS__);       \
        } while (0)

/* NOTE: The local copy of _logname is needed due rkb_logname_lock lock-ordering
 *       when logging another broker's name in the message. */
#define rd_rkb_log(rkb,level,fac,...) do {				\
		rd_kafka_log0(&(rkb)->rkb_rk->rk_conf, \
                              (rkb)->rkb_rk, NULL,                      \
                              level, fac, __VA_ARGS__);                 \
        } while (0)

#define rd_rkb_dbg(rkb,ctx,fac,...) do {				\
		if (unlikely((rkb)->rkb_rk->rk_conf.debug &		\
			     (RD_KAFKA_DBG_ ## ctx))) {			\
			rd_rkb_log(rkb, LOG_DEBUG, fac, __VA_ARGS__);	\
                }                                                       \
	} while (0)

#endif /* _RDKAFKA_INT_H_ */
