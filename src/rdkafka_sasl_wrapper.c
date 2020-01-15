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

#include "rdkafka_int.h"
#include "rdkafka_transport.h"
#include "rdkafka_transport_int.h"
#include "rdkafka_sasl_int.h"
#include "rdkafka_sasl_wrapper.h"



int rd_kafka_sasl_send (rd_kafka_transport_t *rktrans,
                        const void *payload, int len,
                        char *errstr, size_t errstr_size) {
        rd_kafka_broker_t *rkb = rktrans->rktrans_rkb;

        rd_rkb_dbg(rkb, SECURITY, "SASL",
                   "SASL client frame ready (%d bytes)",
                   len);

        if (len == 0)
                return 0;

        if (rktrans->rktrans_clientout.of != 0) {
                rd_rkb_dbg(rktrans->rktrans_rkb, SECURITY, "SASL",
                           "SASL has leftover data (%d bytes)",
                           (int)rktrans->rktrans_clientout.of);
                return -1;
        }

        if (len < 0 || (size_t)len > rktrans->rktrans_clientout.size) {
                rd_rkb_dbg(rktrans->rktrans_rkb, SECURITY, "SASL",
                           "SASL send buffer to small (%d < %d bytes)",
                           (int)rktrans->rktrans_clientout.size, len);
                return -1;
        }

        if (!rktrans->rktrans_clientout.buf) {
                rd_rkb_dbg(rktrans->rktrans_rkb, SECURITY, "SASL",
                           "SASL send missing buffer (%d bytes)",
                           len);
                return -1;
        }

        memcpy(rktrans->rktrans_clientout.buf, payload, len);
        rktrans->rktrans_clientout.of = len;

        return 0;
}


/**
 * @brief Authentication succesful
 *
 * Transition to next connect state.
 */
void rd_kafka_sasl_auth_done (rd_kafka_transport_t *rktrans) {
        /* Authenticated */
        rktrans->authenticated = 1;
}


/**
 * @brief Handle SASL auth data from broker.
 *
 * @locality broker thread
 *
 * @returns -1 on error, 0 if done, 1 if must continue.
 */
int rd_kafka_sasl_step (rd_kafka_transport_t *rktrans,
                        const void *serverin, size_t serverin_size,
                        char *clientout, size_t *clientout_size,
                        char *errstr, size_t errstr_size) {
        int r;

        rd_rkb_dbg(rktrans->rktrans_rkb, SECURITY, "SASL",
                   "Processing SASL frame from server (%"PRIusz" bytes)",
                   serverin_size);

        rktrans->authenticated = 0;
        rktrans->rktrans_clientout.buf = clientout;
        rktrans->rktrans_clientout.size = *clientout_size;
        rktrans->rktrans_clientout.of = 0;

        r = rktrans->rktrans_rkb->rkb_rk->
                rk_conf.sasl.provider->recv(rktrans, serverin, serverin_size,
                                            errstr, errstr_size);

        *clientout_size = rktrans->rktrans_clientout.of;

        return rktrans->authenticated ? 0 : 1;
}



/**
 * @brief Close SASL session (from transport code)
 * @remark May be called on non-SASL transports (no-op)
 */
void rd_kafka_sasl_close (rd_kafka_transport_t *rktrans) {
        const struct rd_kafka_sasl_provider *provider =
                rktrans->rktrans_rkb->rkb_rk->rk_conf.
                sasl.provider;

        if (provider && provider->close)
                provider->close(rktrans);
}



/**
 * Initialize and start SASL authentication.
 *
 * Returns 0 on successful init and -1 on error.
 *
 * Locality: broker thread
 */
int rd_kafka_sasl_client_new (rd_kafka_transport_t *rktrans,
                              const char *hostname,
                              char *clientout, size_t *clientout_size,
                              char *errstr, size_t errstr_size) {
        int r;
        rd_kafka_broker_t *rkb = rktrans->rktrans_rkb;
        rd_kafka_t *rk = rkb->rkb_rk;
        /* char *t; */
        const struct rd_kafka_sasl_provider *provider =
                rk->rk_conf.sasl.provider;

        rktrans->authenticated = 0;
        rktrans->rktrans_clientout.buf = clientout;
        rktrans->rktrans_clientout.size = *clientout_size;
        rktrans->rktrans_clientout.of = 0;

        rd_rkb_dbg(rkb, SECURITY, "SASL",
                   "Initializing SASL client: service name %s, "
                   "hostname %s, mechanisms %s, provider %s",
                   rk->rk_conf.sasl.service_name, hostname,
                   rk->rk_conf.sasl.mechanisms,
                   provider->name);

        r = provider->client_new(rktrans, hostname, errstr, errstr_size);

        *clientout_size = rktrans->rktrans_clientout.of;

        return r;
}



/**
 * @brief Select SASL provider for configured mechanism (singularis)
 * @returns 0 on success or -1 on failure.
 */
static int rd_kafka_sasl_select_provider (rd_kafka_t *rk,
                                          char *errstr, size_t errstr_size) {
        const struct rd_kafka_sasl_provider *provider = NULL;

        if (!strcmp(rk->rk_conf.sasl.mechanisms, "GSSAPI")) {
                /* GSSAPI / Kerberos */
                provider = &rd_kafka_sasl_win32_provider;
        }

        if (!provider) {
                rd_snprintf(errstr, errstr_size,
                            "No provider for SASL mechanism %s; "
                            "Available: WindowsSSPI(GSSAPI)",
                            rk->rk_conf.sasl.mechanisms);
                return -1;
        }

        rd_kafka_dbg(rk, SECURITY, "SASL",
                     "Selected provider %s for SASL mechanism %s",
                     provider->name, rk->rk_conf.sasl.mechanisms);

        /* Validate SASL config */
        if (provider->conf_validate &&
            provider->conf_validate(rk, errstr, errstr_size) == -1)
                return -1;

        rk->rk_conf.sasl.provider = provider;

        return 0;
}



rd_kafka_transport_t *rd_kafka_sasl_wrapper_new (const char *mechanisms,
                                                 const char *service_name,
                                                 rd_kafka_log_cb_t *log_cb,
                                                 char *errstr,
                                                 size_t errstr_size) {
        rd_kafka_t *rk = rd_calloc(1, sizeof(*rk));
        rd_kafka_transport_t *rktrans = rd_calloc(1, sizeof(*rktrans));
        rd_kafka_broker_t *rkb = rd_calloc(1, sizeof(*rkb));
        int r;

        rkb->rkb_rk = rk;
        rkb->rkb_features = RD_KAFKA_FEATURE_SASL_AUTH_REQ;

        rktrans->rktrans_rkb = rkb;

        rk->rk_conf.log_cb = log_cb;
        rk->rk_conf.log_level = LOG_DEBUG;
        rk->rk_conf.debug = RD_KAFKA_DBG_ALL;

        rk->rk_conf.sasl.mechanisms = rd_strdup(mechanisms);
        rk->rk_conf.sasl.service_name = rd_strdup(service_name);

        r = rd_kafka_sasl_select_provider(rk, errstr, errstr_size);

        if (r != 0) {
                rd_kafka_sasl_wrapper_free(rktrans);

                return NULL;
        }

        return rktrans;
}



void rd_kafka_sasl_wrapper_free(rd_kafka_transport_t *rktrans) {
        rd_kafka_broker_t *rkb = rktrans->rktrans_rkb;
        rd_kafka_t *rk = rkb->rkb_rk;

        rd_free(rk->rk_conf.sasl.mechanisms);
        rd_free(rk->rk_conf.sasl.service_name);
        rd_free(rk);

        rd_free(rkb);

        rd_free(rktrans);
}
