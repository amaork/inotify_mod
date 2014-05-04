#ifndef	_INOTIFY_MOD_H_
#define	_INOTIFY_MOD_H_

/* XXX:Included */
#include "iniparser/iniparser.h"

/* XXX:Macro */
#define DEF_ASCII	1	
#define	DEF_PORT	9000
#define	DEF_ADDR	"127.0.0.1"
#define	DEF_CONF_FILE	"./inotify_mod.ini"


#define	PATH_KEY	"path"
#define IGNORE_KEY	"ignore"
#define	EVENTS_KEY	"events"
#define SPECIAL_KEY	"special"
#define	COMMENT_KEY	"comment"

/* XXX:Data structure */

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
	#define	ADD_EVENT	"add"
	#define	DEL_EVENT	"del"
	#define	MOD_EVENT	"mod"
	#define ADD_MASK	(1 << 0)
	#define DEL_MASK	(1 << 1)
	#define MOD_MASK	(1 << 2)
	#define IS_ADD_SET(x)	((x) & ADD_MASK)
	#define IS_DEL_SET(x)	((x) & DEL_MASK)
	#define IS_MOD_SET(x)	((x) & MOD_MASK)
}WATCH_INFO, *P_WATCH_INFO;


/* Inotify module communication init data structure, send message to outside used too */
typedef	struct{
	
	int socket;						/*	Communication socket fd */
	struct sockaddr_in addr;		/*	Cache target address */
	unsigned int is_ascii;			/*	Message data format, if it set using ascii format  */
}COMM_INFO, *P_COMM_INFO;


/* Watch event message binary data structure */
typedef	struct{
	
	char name[32];					/*	Watch name (ini section name )*/
	char path[32];					/*	File name */
	unsigned int idx;				/*	Conf index */
	unsigned int events;			/*	Which event has happend */
	unsigned char spc[MAX_SPC_FILE];/* 	Special file has add or remove */
}MSG_INFO, *P_MSG_INFO;


/* XXX:Function declare  */

/* Read communication params form configure file and init socket */
static int comm_init(dictionary *conf, P_COMM_INFO comm);

/* Read and parser configure file, cache the configure args to WATCH_INFO */
static int conf_init(dictionary *conf, P_WATCH_INFO watch, const unsigned int size);

/* Init a inotify instance, and add the file to the inotify instance */
static int watch_init(P_WATCH_INFO watch_list, unsigned int size);

/* Send events message */
static int send_msg(P_COMM_INFO, P_MSG_INFO);

/* Print help message */
static void print_help(int argc, char **argv);
#endif
