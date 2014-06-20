#ifndef	_COMM_H_
#define _COMM_H_

#include "inotify_mod.h"

/* Read communication params form configure file and init socket */
int comm_init(dictionary *conf, P_COMM_INFO comm);


/* Send events message */
int comm_send_msg(P_COMM_INFO comm, P_MSG_INFO msg);

#endif
