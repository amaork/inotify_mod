#ifndef	_CONFIGURE_H_
#define _CONFIGURE_H_

#include "inotify_mod.h"

#define IS_WORD_END(x)	((x) == ' ' || (x) == '\t' || (x) == '\n' || (x) == ',' || (x) == ']' || (x) == '[')	

/* Check if path is a dir */
int conf_is_path_is_dir(char *path);

/* Print watch infomaction */
void conf_print_watch(const P_WATCH_INFO watch);

/* Get words form cp string */
int conf_get_words(const char *cp, char (*words)[32], unsigned int size);

/* Read configure file to watch */
int conf_init(dictionary *conf, P_WATCH_INFO watch, const unsigned int size);
#endif
