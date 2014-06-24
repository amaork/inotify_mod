#ifndef	_CONFIGURE_H_
#define _CONFIGURE_H_

#include "inotify_mod.h"

#define IS_WORD_END(x)	((x) == ' ' || (x) == '\t' || (x) == '\n' || (x) == ',' || (x) == ']')	

/* Read and parser configure file, cache the configure args to WATCH_INFO */
int conf_is_path_is_dir(char *path);
void conf_print_watch(const P_WATCH_INFO watch);
int conf_init(dictionary *conf, P_WATCH_INFO watch, const unsigned int size);

#endif
