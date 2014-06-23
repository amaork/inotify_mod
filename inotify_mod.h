#ifndef	_INOTIFY_MOD_H_
#define	_INOTIFY_MOD_H_

/* XXX:Included */
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/inotify.h>
#include "debug.h"
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
	#define READ_EVENT	"read"
	#define SDEL_EVENT	"sdel"
	#define ADD_MASK	(1 << 0)
	#define DEL_MASK	(1 << 1)
	#define MOD_MASK	(1 << 2)
	#define READ_MASK	(1 << 3)
	#define SDEL_MASK	(1 << 4)
	#define IS_ADD_SET(x)	((x) & ADD_MASK)
	#define IS_DEL_SET(x)	((x) & DEL_MASK)
	#define IS_MOD_SET(x)	((x) & MOD_MASK)
	#define IS_READ_SET(x)	((x) & READ_MASK)
	#define IS_SDEL_SET(x)	((x) & SDEL_MASK)
}WATCH_INFO, *P_WATCH_INFO;


extern char *conf_file_path;	

#endif
