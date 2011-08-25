/*
 * Copyright (C)  2011 - David Goulet <david.goulet@polymtl.ca>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; only version 2
 * of the License.
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

#define _GNU_SOURCE
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <lttng-sessiond-comm.h>

/*
 * Human readable error message.
 */
static const char *lttcomm_readable_code[] = {
	[ LTTCOMM_ERR_INDEX(LTTCOMM_OK) ] = "Success",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_ERR) ] = "Unknown error",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_UND) ] = "Undefined command",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_NOT_IMPLEMENTED) ] = "Not implemented",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_UNKNOWN_DOMAIN) ] = "Unknown tracing domain",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_NO_SESSION) ] = "No session found",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_LIST_FAIL) ] = "Unable to list traceable apps",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_NO_APPS) ] = "No traceable apps found",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_SESS_NOT_FOUND) ] = "Session name not found",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_NO_TRACE) ] = "No trace found",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_FATAL) ] = "Fatal error of the session daemon",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_CREATE_FAIL) ] = "Create trace failed",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_START_FAIL) ] = "Start trace failed",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_STOP_FAIL) ] = "Stop trace failed",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_NO_TRACEABLE) ] = "App is not traceable",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_SELECT_SESS) ] = "A session MUST be selected",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_EXIST_SESS) ] = "Session name already exist",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_CONNECT_FAIL) ] = "Unable to connect to Unix socket",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_KERN_NA) ] = "Kernel tracer not available",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_KERN_EVENT_EXIST) ] = "Kernel event already exists",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_KERN_SESS_FAIL) ] = "Kernel create session failed",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_KERN_CHAN_FAIL) ] = "Kernel create channel failed",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_KERN_CHAN_NOT_FOUND) ] = "Kernel channel not found",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_KERN_CHAN_DISABLE_FAIL) ] = "Disable kernel channel failed",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_KERN_CHAN_ENABLE_FAIL) ] = "Enable kernel channel failed",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_KERN_CONTEXT_FAIL) ] = "Add kernel context failed",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_KERN_ENABLE_FAIL) ] = "Enable kernel event failed",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_KERN_DISABLE_FAIL) ] = "Disable kernel event failed",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_KERN_META_FAIL) ] = "Opening metadata failed",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_KERN_START_FAIL) ] = "Starting kernel trace failed",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_KERN_STOP_FAIL) ] = "Stoping kernel trace failed",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_KERN_CONSUMER_FAIL) ] = "Kernel consumer start failed",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_KERN_STREAM_FAIL) ] = "Kernel create stream failed",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_KERN_DIR_FAIL) ] = "Kernel trace directory creation failed",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_KERN_DIR_EXIST) ] = "Kernel trace directory already exist",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_KERN_NO_SESSION) ] = "No kernel session found",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_KERN_LIST_FAIL) ] = "Listing kernel events failed",
	[ LTTCOMM_ERR_INDEX(KCONSUMERD_COMMAND_SOCK_READY) ] = "Kconsumerd command socket ready",
	[ LTTCOMM_ERR_INDEX(KCONSUMERD_SUCCESS_RECV_FD) ] = "Kconsumerd success on receiving fds",
	[ LTTCOMM_ERR_INDEX(KCONSUMERD_ERROR_RECV_FD) ] = "Kconsumerd error on receiving fds",
	[ LTTCOMM_ERR_INDEX(KCONSUMERD_POLL_ERROR) ] = "Kconsumerd error in polling thread",
	[ LTTCOMM_ERR_INDEX(KCONSUMERD_POLL_NVAL) ] = "Kconsumerd polling on closed fd",
	[ LTTCOMM_ERR_INDEX(KCONSUMERD_POLL_HUP) ] = "Kconsumerd all fd hung up",
	[ LTTCOMM_ERR_INDEX(KCONSUMERD_EXIT_SUCCESS) ] = "Kconsumerd exiting normally",
	[ LTTCOMM_ERR_INDEX(KCONSUMERD_EXIT_FAILURE) ] = "Kconsumerd exiting on error",
	[ LTTCOMM_ERR_INDEX(KCONSUMERD_OUTFD_ERROR) ] = "Kconsumerd error opening the tracefile",
	[ LTTCOMM_ERR_INDEX(KCONSUMERD_SPLICE_EBADF) ] = "Kconsumerd splice EBADF",
	[ LTTCOMM_ERR_INDEX(KCONSUMERD_SPLICE_EINVAL) ] = "Kconsumerd splice EINVAL",
	[ LTTCOMM_ERR_INDEX(KCONSUMERD_SPLICE_ENOMEM) ] = "Kconsumerd splice ENOMEM",
	[ LTTCOMM_ERR_INDEX(KCONSUMERD_SPLICE_ESPIPE) ] = "Kconsumerd splice ESPIPE",
	[ LTTCOMM_ERR_INDEX(LTTCOMM_NO_EVENT) ] = "Event not found",
};

/*
 *  lttcom_get_readable_code
 *
 *  Return ptr to string representing a human readable
 *  error code from the lttcomm_return_code enum.
 *
 *  These code MUST be negative in other to treat that
 *  as an error value.
 */
const char *lttcomm_get_readable_code(enum lttcomm_return_code code)
{
	int tmp_code = -code;

	if (tmp_code >= LTTCOMM_OK && tmp_code < LTTCOMM_NR) {
		return lttcomm_readable_code[LTTCOMM_ERR_INDEX(tmp_code)];
	}

	return "Unknown error code";
}

/*
 * 	lttcomm_connect_unix_sock
 *
 * 	Connect to unix socket using the path name.
 */
int lttcomm_connect_unix_sock(const char *pathname)
{
	struct sockaddr_un sun;
	int fd;
	int ret;

	fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket");
		ret = fd;
		goto error;
	}

	memset(&sun, 0, sizeof(sun));
	sun.sun_family = AF_UNIX;
	strncpy(sun.sun_path, pathname, sizeof(sun.sun_path));
	sun.sun_path[sizeof(sun.sun_path) - 1] = '\0';

	ret = connect(fd, (struct sockaddr *) &sun, sizeof(sun));
	if (ret < 0) {
		/*
		 * Don't print message on connect error, because connect
		 * is used in normal execution to detect if sessiond is
		 * alive.
		 */
		goto error_connect;
	}

	return fd;

error_connect:
	close(fd);
error:
	return ret;
}

/*
 * 	lttcomm_accept_unix_sock
 *
 *	Do an accept(2) on the sock and return the
 *	new file descriptor. The socket MUST be bind(2) before.
 */
int lttcomm_accept_unix_sock(int sock)
{
	int new_fd;
	struct sockaddr_un sun;
	socklen_t len = 0;

	/* Blocking call */
	new_fd = accept(sock, (struct sockaddr *) &sun, &len);
	if (new_fd < 0) {
		perror("accept");
		goto error;
	}

	return new_fd;

error:
	return -1;
}

/*
 * 	lttcomm_create_unix_sock
 *
 * 	Creates a AF_UNIX local socket using pathname
 * 	bind the socket upon creation and return the fd.
 */
int lttcomm_create_unix_sock(const char *pathname)
{
	struct sockaddr_un sun;
	int fd;
	int ret = -1;

	/* Create server socket */
	if ((fd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		goto error;
	}

	memset(&sun, 0, sizeof(sun));
	sun.sun_family = AF_UNIX;
	strncpy(sun.sun_path, pathname, sizeof(sun.sun_path));
	sun.sun_path[sizeof(sun.sun_path) - 1] = '\0';

	/* Unlink the old file if present */
	(void) unlink(pathname);
	ret = bind(fd, (struct sockaddr *) &sun, sizeof(sun));
	if (ret < 0) {
		perror("bind");
		goto error;
	}

	return fd;

error:
	return ret;
}

/*
 * 	lttcomm_listen_unix_sock
 *
 * 	Make the socket listen using MAX_LISTEN.
 */
int lttcomm_listen_unix_sock(int sock)
{
	int ret;

	ret = listen(sock, MAX_LISTEN);
	if (ret < 0) {
		perror("listen");
	}

	return ret;
}

/*
 * 	lttcomm_recv_unix_sock
 *
 *  Receive data of size len in put that data into
 *  the buf param. Using recvmsg API.
 *  Return the size of received data.
 */
ssize_t lttcomm_recv_unix_sock(int sock, void *buf, size_t len)
{
	struct msghdr msg = { 0 };
	struct iovec iov[1];
	ssize_t ret = -1;

	iov[0].iov_base = buf;
	iov[0].iov_len = len;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	ret = recvmsg(sock, &msg, 0);
	if (ret < 0) {
		perror("recvmsg");
	}

	return ret;
}

/*
 * 	lttcomm_send_unix_sock
 *
 * 	Send buf data of size len. Using sendmsg API.
 * 	Return the size of sent data.
 */
ssize_t lttcomm_send_unix_sock(int sock, void *buf, size_t len)
{
	struct msghdr msg = { 0 };
	struct iovec iov[1];
	ssize_t ret = -1;

	iov[0].iov_base = buf;
	iov[0].iov_len = len;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	ret = sendmsg(sock, &msg, 0);
	if (ret < 0) {
		perror("sendmsg");
	}

	return ret;
}

/*
 *  lttcomm_close_unix_sock
 *
 *  Shutdown cleanly a unix socket.
 */
int lttcomm_close_unix_sock(int sock)
{
	int ret;

	/* Shutdown receptions and transmissions */
	ret = shutdown(sock, SHUT_RDWR);
	if (ret < 0) {
		perror("shutdown");
	}

	return ret;
}

/*
 *  lttcomm_send_fds_unix_sock
 *
 *  Send multiple fds on a unix socket.
 */
ssize_t lttcomm_send_fds_unix_sock(int sock, void *buf, int *fds, size_t nb_fd, size_t len)
{
	struct msghdr msg = { 0 };
	struct cmsghdr *cmptr;
	struct iovec iov[1];
	ssize_t ret = -1;
	unsigned int sizeof_fds = nb_fd * sizeof(int);
	char tmp[CMSG_SPACE(sizeof_fds)];

	/*
	 * Note: the consumerd receiver only supports receiving one FD per
	 * message.
	 */
	assert(nb_fd == 1);

	msg.msg_control = (caddr_t)tmp;
	msg.msg_controllen = CMSG_LEN(sizeof_fds);

	cmptr = CMSG_FIRSTHDR(&msg);
	cmptr->cmsg_level = SOL_SOCKET;
	cmptr->cmsg_type = SCM_RIGHTS;
	cmptr->cmsg_len = CMSG_LEN(sizeof_fds);
	memcpy(CMSG_DATA(cmptr), fds, sizeof_fds);
	/* Sum of the length of all control messages in the buffer: */
	msg.msg_controllen = cmptr->cmsg_len;

	iov[0].iov_base = buf;
	iov[0].iov_len = len;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	ret = sendmsg(sock, &msg, 0);
	if (ret < 0) {
		perror("sendmsg");
	}

	return ret;
}