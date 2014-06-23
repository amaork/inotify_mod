#ifndef	_COMM_H_
#define _COMM_H_

#include "inotify_mod.h"


/* Inotify module communication init data structure, send message to outside user too */
typedef	struct{
	int socket;						/*	Communication socket fd */
	unsigned int is_ascii;			/*	Message data format, if it set using ascii format  */
	struct sockaddr_in addr;		/*	Cache target address */
}COMM_INFO, *P_COMM_INFO;

/* Watch event message binary data structure */
typedef	struct{
	
	char name[32];					/*	Watch name (ini section name )*/
	char path[128];					/*	File full path */
	unsigned int idx;				/*	Conf index */
	unsigned int is_dir;			/*	If it was a dir it will set */
	unsigned int events;			/*	Which event has happend */
	unsigned char spc[MAX_SPC_FILE];/* 	Special file has add or remove */
}MSG_INFO, *P_MSG_INFO;


/* Read communication params form configure file and init socket */
int comm_init(dictionary *conf);


/* Set MSG_INFO field */
inline void comm_set_msg_name(P_MSG_INFO msg, char *name);
inline void comm_set_msg_path(P_MSG_INFO msg, char *path);
inline void comm_set_msg_dir(P_MSG_INFO msg, unsigned int mask);

/* Send events message */
int comm_send_msg(P_MSG_INFO msg);

#endif
