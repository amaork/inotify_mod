#ifndef	_WATCH_H_
#define _WATCH_H_

#include "inotify_mod.h"

/* Inotify_mod.ini section data structure, one section one WATCH_INFO */
typedef	struct{

	int fd;							/*	When it add to inotify cache fd */
	char name[32];					/*	Section name such as [CONF ]*/
	char path[128];					/*	Watch dir or file path */
	char comment[128];				/*	Watch comment */
	unsigned int ignore;			/*	Don't add this watch */
	unsigned int events;			/*	Watch events */
	unsigned int is_dir;			/*	If it was a dir it will set */
	#define MAX_SPC_FILE	8
	char spc_name[MAX_SPC_FILE][32];/*	Special file name */
	unsigned int spc_name_cnt;		/*	How man special file have set */
}WATCH_INFO, *P_WATCH_INFO;


/* Init a inotify instance, and add the file to the inotify instance */
int watch_init(P_WATCH_INFO watch_list, unsigned int size);

#endif
