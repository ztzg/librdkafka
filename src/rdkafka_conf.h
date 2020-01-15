/*
 * librdkafka - Apache Kafka C library
 *
 * Copyright (c) 2014-2018 Magnus Edenhill
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

#ifndef _RDKAFKA_CONF_H_
#define _RDKAFKA_CONF_H_

/**
 * Forward declarations
 */
struct rd_kafka_transport_s;

/**
 * Optional configuration struct passed to rd_kafka_new*().
 *
 * The struct is populated ted through string properties
 * by calling rd_kafka_conf_set().
 *
 */
struct rd_kafka_conf_s {
	int     debug;
        struct {
                const struct rd_kafka_sasl_provider *provider;
                char *principal;
                char *mechanisms;
                char *service_name;
                char *kinit_cmd;
                char *keytab;
                int   relogin_min_time;
                char *username;
                char *password;
        } sasl;

        /* Log callback */
        void (*log_cb) (const rd_kafka_t *rk, int level,
                        const char *fac, const char *buf);
        int    log_level;
};

#endif /* _RDKAFKA_CONF_H_ */
