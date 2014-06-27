#ifndef	_WATCH_H_
#define _WATCH_H_

#include "inotify_mod.h"
#include "comm.h"

/* Inotify_mod.ini section data structure, one section one WATCH_INFO */
typedef	struct{

	int fd;							/*	When it add to inotify cache fd */
	char name[32];					/*	Section name such as [CONF ]*/
	char path[128];					/*	Watch dir or file path */
	char comment[128];				/*	Watch comment */
	int is_dir;						/*	If it was a dir it will set */
	unsigned int ignore;			/*	Don't add this watch */
	unsigned int events;			/*	Watch events */
	char spc_file[MAX_SPC_FILE][32];/*	Special file name */
	unsigned int spc_file_cnt;		/*	How man special file have set */
	unsigned int dynamic_watch;		/*	Dynamic watch flag */
	char events_str[128];			/*	Events string, dynamic watch using */ 
}WATCH_INFO, *P_WATCH_INFO;

/* Remove watch from inotify watching list */
int watch_remove(P_WATCH_INFO wlist, int size, int idx);

/* Init a inotify instance, and add the file to the inotify instance */
int watch_init(P_WATCH_INFO watch_list, unsigned int size);

/* Check if the event is watch want watching event */
int watch_event_check(const P_WATCH_INFO watch, struct inotify_event *event, P_MSG_INFO msg);

/* Check if it dynamic add event */
int watch_dynamic_check(int watch_fd, const P_WATCH_INFO watch, struct inotify_event *event);

#endif
