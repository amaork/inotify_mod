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

#define MAX_SPC_FILE	8

/* XXX:Data structure */
extern char *conf_file_path;	

#endif
