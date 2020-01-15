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


#define _GNU_SOURCE
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "rdkafka_int.h"

#ifdef _MSC_VER
#include <sys/types.h>
#include <sys/timeb.h>
#endif

static void rd_kafka_log_buf (const rd_kafka_conf_t *conf,
                              const rd_kafka_t *rk, int level, const char *fac,
                              const char *buf) {
        if (level > conf->log_level)
                return;
        if (conf->log_cb)
                conf->log_cb(rk, level, fac, buf);
}

/**
 * @brief Logger
 *
 * @remark conf must be set, but rk may be NULL
 */
void rd_kafka_log0 (const rd_kafka_conf_t *conf,
                    const rd_kafka_t *rk,
                    const char *extra, int level,
                    const char *fac, const char *fmt, ...) {
	char buf[2048];
	va_list ap;
	unsigned int elen = 0;
        unsigned int of = 0;

	if (level > conf->log_level)
		return;

#if 0
	if (conf->log_thread_name) {
		elen = rd_snprintf(buf, sizeof(buf), "[thrd:%s]: ",
				   rd_kafka_thread_name);
		if (unlikely(elen >= sizeof(buf)))
			elen = sizeof(buf);
		of = elen;
	}
#endif  /* 0 */

	if (extra) {
		elen = rd_snprintf(buf+of, sizeof(buf)-of, "%s: ", extra);
		if (unlikely(elen >= sizeof(buf)-of))
			elen = sizeof(buf)-of;
                of += elen;
	}

	va_start(ap, fmt);
	rd_vsnprintf(buf+of, sizeof(buf)-of, fmt, ap);
	va_end(ap);

        rd_kafka_log_buf(conf, rk, level, fac, buf);
}
