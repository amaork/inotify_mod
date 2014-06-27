#ifndef	_EVENTS_H_
#define _EVENTS_H_

#include "inotify_mod.h"

#define ALL_EVENT	"all"
#define	ADD_EVENT	"add"
#define	DEL_EVENT	"del"
#define	MOD_EVENT	"mod"
#define READ_EVENT	"read"
#define SDEL_EVENT	"sdel"
#define ATTR_EVENT	"attr"

#define ADD_MASK	(1 << 0)
#define DEL_MASK	(1 << 1)
#define MOD_MASK	(1 << 2)
#define READ_MASK	(1 << 3)
#define SDEL_MASK	(1 << 4)
#define ATTR_MASK	(1 << 5)
#define ALL_MASK	(1 << 31)

/* File and dir support events mask */
#define FILE_SUPPORT_EVENTS	(MOD_MASK | READ_MASK | SDEL_MASK | ATTR_MASK)
#define DIR_SUPPORT_EVENTS	(ADD_MASK | DEL_MASK | MOD_MASK | SDEL_MASK | ATTR_MASK)

#define I_ADD_MASK	IN_CREATE
#define I_DEL_MASK	IN_DELETE
#define I_MOD_MASK	IN_CLOSE_WRITE
#define I_READ_MASK	IN_ACCESS
#define I_SDEL_MASK	IN_DELETE_SELF
#define I_ATTR_MASK	IN_ATTRIB

/* Check is event set */
#define IS_ADD_SET(x)	((x) & ADD_MASK)
#define IS_DEL_SET(x)	((x) & DEL_MASK)
#define IS_MOD_SET(x)	((x) & MOD_MASK)
#define IS_READ_SET(x)	((x) & READ_MASK)
#define IS_SDEL_SET(x)	((x) & SDEL_MASK)
#define IS_ATTR_SET(x)	((x) & ATTR_MASK)
#define IS_ALL_SET(x)	((x) & ALL_MASK)


/* Check if support this events */
#define IS_DIR_SUPPORT(x)		((x) & (DIR_SUPPORT_EVENTS))
#define IS_FILE_SUPPORT(x)		((x) & (FILE_SUPPORT_EVENTS))


#define MAX_EVENT_SIZE	12

typedef	struct{

	char *name;				/* 	Watch event name */
	unsigned int mask;		/*	Watch event mask */
	unsigned int imask;		/*	Inotify event mask */
}WT_EVENT, *P_WT_EVENT;


extern const WT_EVENT support_events[];


char *ev_get_events_str(unsigned int events, char *buf, int size);
inline int ev_is_support_event(unsigned int is_dir, unsigned int  event);

#endif
