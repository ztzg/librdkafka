/*
 * librdkafka - The Apache Kafka C/C++ library
 *
 * Copyright (c) 2015 Magnus Edenhill
 * Copyright (c) 2020 Damien Diederen
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

#ifndef _RDKAFKA_SASL_WRAPPER_H_
#define _RDKAFKA_SASL_WRAPPER_H_

#include "rdkafka.h"

typedef void rd_kafka_log_cb_t (const rd_kafka_t *rk, int level,
                                const char *fac, const char *buf);

RD_EXPORT
rd_kafka_transport_t *rd_kafka_sasl_wrapper_new (const char *mechanisms,
                                                 const char *service_name,
                                                 rd_kafka_log_cb_t *log_cb,
                                                 char *errstr,
                                                 size_t errstr_size);

RD_EXPORT
int rd_kafka_sasl_client_new (rd_kafka_transport_t *rktrans,
                              const char *hostname,
                              char *clientout, size_t *clientout_size,
                              char *errstr, size_t errstr_size);

RD_EXPORT
int rd_kafka_sasl_step (rd_kafka_transport_t *rktrans,
                        const void *serverin, size_t serverin_size,
                        char *clientout, size_t *clientout_size,
                        char *errstr, size_t errstr_size);

RD_EXPORT
void rd_kafka_sasl_close (rd_kafka_transport_t *rktrans);

RD_EXPORT
void rd_kafka_sasl_wrapper_free (rd_kafka_transport_t *rktrans);

#endif /* _RDKAFKA_SASL_WRAPPER_H_ */
