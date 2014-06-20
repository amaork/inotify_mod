#ifndef	_CONFIGURE_H_
#define _CONFIGURE_H_

#include "inotify_mod.h"

/* Read and parser configure file, cache the configure args to WATCH_INFO */
int conf_init(dictionary *conf, P_WATCH_INFO watch, const unsigned int size);

#endif
