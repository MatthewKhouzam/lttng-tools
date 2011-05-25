/*
 * Copyright (C) 2011 - David Goulet <david.goulet@polymtl.ca>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef _LTT_SESSIOND_H
#define _LTT_SESSIOND_H

#define DEFAULT_HOME_DIR			"/tmp"
#define DEFAULT_UST_SOCK_DIR		DEFAULT_HOME_DIR "/ust-app-socks"
#define DEFAULT_GLOBAL_APPS_PIPE	DEFAULT_UST_SOCK_DIR "/global"
#define DEFAULT_TRACE_OUTPUT        DEFAULT_HOME_DIR "/lttng"

/*
 * Kernel tracer defines
 */
#define DEFAULT_KERNEL_TRACER_PATH			"/mnt/debugfs/lttng"
#define DEFAULT_KERNEL_OVERWRITE            0
#define DEFAULT_KERNEL_SUBBUF_SIZE          4096    /* bytes */
#define DEFAULT_KERNEL_SUBBUF_NUM           8       /* Must always be a power of 2 */
#define DEFAULT_KERNEL_SWITCH_TIMER         0       /* usec */
#define DEFAULT_KERNEL_READ_TIMER           200     /* usec */

extern const char default_home_dir[],
	default_tracing_group[],
	default_ust_sock_dir[],
	default_global_apps_pipe[];

/*
 * This contains extra data needed for processing a command received by the
 * session daemon from the lttng client.
 */
struct command_ctx {
	int ust_sock;
	unsigned int lttng_msg_size;
	struct ltt_session *session;
	struct lttcomm_lttng_msg *llm;
	struct lttcomm_session_msg *lsm;
};

#endif /* _LTT_SESSIOND_H */
