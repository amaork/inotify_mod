#ifndef	_WATCH_H_
#define _WATCH_H_

#include "inotify_mod.h"

/* Init a inotify instance, and add the file to the inotify instance */
int watch_init(P_WATCH_INFO watch_list, unsigned int size);

#endif
