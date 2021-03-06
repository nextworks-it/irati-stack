/*
 * DTCP (Data Transfer Control Protocol)
 *
 *    Francesco Salvestrini <f.salvestrini@nextworks.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef RINA_DTCP_H
#define RINA_DTCP_H

#include "common.h"
#include "du.h"
#include "dt.h"

struct dtcp_config;
struct dtcp_fctrl_config;
struct dtcp_rctrl_config;
struct connection;
struct rmt;

struct dtcp *        dtcp_create(struct dt *         dt,
                                 struct connection * conn,
                                 struct rmt *        rmt);

int                  dtcp_destroy(struct dtcp * instance);

int                  dtcp_send(struct dtcp * instance,
                               struct sdu *  sdu);

/* Used by the DTP to notify the DTCP about events */
int                  dtcp_sv_update(struct dtcp * instance,
                                    seq_num_t     seq);

/* Used by EFCP to send an incoming DTCP PDU */
int                  dtcp_common_rcv_control(struct dtcp * dtcp,
                                             struct pdu *  pdu);

/* Used by DTP to have an ack-control PDU sent by DTCP */
int                  dtcp_ack_flow_control_pdu_send(struct dtcp * instance,
                                                    seq_num_t     seq);

seq_num_t            dtcp_rcv_rt_win(struct dtcp * instance);
seq_num_t            dtcp_snd_rt_win(struct dtcp * instance);
seq_num_t            dtcp_snd_lf_win(struct dtcp * instance);
int                  dtcp_snd_lf_win_set(struct dtcp * instance,
                                         seq_num_t seq_num);

struct dtcp_config * dtcp_config_get(struct dtcp * dtcp);

/*FIXME: wrapper to be called by dtp in the post_worker */
int                  dtcp_sending_ack_policy(struct dtcp * dtcp);
#endif
