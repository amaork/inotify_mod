#include <libgen.h>
#include "comm.h"
#include "watch.h"
#include "events.h"
#include "configure.h"

#define MAX_WATCH_SIZE	256

struct ref_count{

	unsigned int size;
	unsigned short cnt[MAX_WATCH_SIZE];
};

static struct ref_count dynamic_watch_ref = {.size = 0, .cnt = {0}};

/*************************************************************************************************************
**	dynamic watch get	
*************************************************************************************************************/
static void dynamic_watch_get(int fd)
{
	if (fd < MAX_WATCH_SIZE){
	
		dynamic_watch_ref.cnt[fd]++;
	}
}

static unsigned int dynamic_watch_put(int fd)
{
	if (fd < MAX_WATCH_SIZE){

		dynamic_watch_ref.cnt[fd]--;
		return dynamic_watch_ref.cnt[fd];	
	}

	return 0;
}



/*************************************************************************************************************
**	@brief	:	analytics  events and get watching mask
**	#events	:	watching events
**	#is_dir	:	is watching is a dir
**	@return	:	return watching mask
*************************************************************************************************************/
static unsigned int watch_get_mask(unsigned int events, unsigned int is_dir)
{
	unsigned int watch_mask = 0;

	/* Add */
	if (IS_ADD_SET(events)){

		watch_mask 	|=	IN_CREATE;
	}

	/* Del */
	if (IS_DEL_SET(events)){

		watch_mask	|=	IN_DELETE;
	}

	/* Mod */
	if (IS_MOD_SET(events)){

		if (is_dir){

			watch_mask	|=	IN_CREATE | IN_DELETE;
		}
		else{
			
			watch_mask	|=	IN_CLOSE_WRITE;
		}
	}

	/* Read */
	if (IS_READ_SET(events)){

		watch_mask	|=	IN_ACCESS;
	}
		

	/* Sdel */
	if (IS_SDEL_SET(events)){

		if (is_dir){

			watch_mask 	|=	IN_IGNORED;
		}
		else{
				
			watch_mask	|=	IN_DELETE_SELF;
		}
	}


	return watch_mask;
}


/*************************************************************************************************************
**	@brief		:	add watch father dir to inotify watch and set this watch as dynamic watch
**	#watch_fd	:	inotify watch fd
**	#watch		:	dynamic watch
**	@return		:	success return dynamic watch fd
*************************************************************************************************************/
static int watch_dynamic_add(unsigned int watch_fd, P_WATCH_INFO watch)
{
	int dynamic_fd = 0;
	char *dir_name = conf_dirname(watch->path);

	/* First check if watch file/directory father dir is exist */	
	if (conf_is_path_is_dir(dir_name) != 1){

		fprintf(stderr, "Add dynamic watch[%s:%s] is failed, father dir is not exist!\n", watch->name, watch->path);
		return -1;
	}
	
	/* If watch file/directory father dir is exist watch father dir Add events */	
	if ((dynamic_fd = inotify_add_watch(watch_fd, dir_name, watch_get_mask(ADD_MASK, 1))) == -1){

		fprintf(stderr, "Add dynamic watch[%s:%s] is failed:%s\n", watch->name, watch->path, strerror(errno));
		return -1;
	}	

	/* Set this watch is dynamc watch */
	watch->fd = dynamic_fd;
	watch->dynamic_watch = 1;
	dynamic_watch_get(dynamic_fd);

	/* Return dynamic watch fd */
	return dynamic_fd;
}



/*************************************************************************************************************
**	@brief		:	delete dynamic watch and add orignal watch	
**	#watch_fd	:	inotify watch fd
**	#watvh		:	dynamic watch
**	@return		:	success return original watch fd
*************************************************************************************************************/
static int watch_dynamic_del(unsigned int watch_fd, P_WATCH_INFO watch)
{
	int original_fd = 0;
	unsigned int watch_mask;

	/* Check if original path is a dir */
	if ((watch->is_dir = conf_is_path_is_dir(watch->path)) == -1){
		
		fprintf(stderr, "Original watch[%s:%s] do not exist!\n", watch->name, watch->path);
		return -1;
	}

	/* According watch attribute(file or dir) re-parser watching event */
	watch->events = conf_parser_events(watch->events_str, watch->is_dir);

	/* Get watching mask */
	watch_mask = watch_get_mask(watch->events, watch->is_dir);

	/* Add original file/dir to inotify watch  */
	if ((original_fd = inotify_add_watch(watch_fd, watch->path, watch_mask)) == -1){

		fprintf(stderr, "Add original watch[%s:%s] failed,%s\n", watch->name, watch->path, strerror(errno));
		return -1;
	}

	/* Delete original file/dir father dir form inotify watch */
	if (dynamic_watch_put(watch->fd) == 0){

		if (inotify_rm_watch(watch_fd, watch->fd) == -1){

			fprintf(stderr, "Delete dynamic watch[%s:%s] is failed, %s\n", watch->name, watch->path, strerror(errno));
		}
		else{
	
			fprintf(stdout, "Delete dynamic watch -> %d\n", watch->fd);		
		}
	}

	/* Clear dyanic warch */
	watch->fd = original_fd;
	watch->dynamic_watch = 0;

	/* Debug */
	if (debug){

		conf_print_watch(watch);
	}


	/* Return watch instance */
	return original_fd;
}


/*************************************************************************************************************
**	@brief		:	Create a inotify instance then add watch_list path to the instace 
**	#watch_list :	user conf data structure 
**	#size		:	watch_list size 
**	@return 	:	success return inotify instace fd falied return -1
*************************************************************************************************************/
int watch_init(P_WATCH_INFO watch_list, unsigned int size)
{
	int watch_fd;	
	unsigned int i;
	unsigned int watch_mask;

	/* Create a inotify instance  */
	if ((watch_fd = inotify_init()) == -1){

		fprintf(stderr, "Create inotify instance error:%s!\n", strerror(errno));
		return -1;
	}

	/* Add file to inotidy watch  */
	for (i = 0; i < size; i++){

		/* Get watch event and set watch mask */
		watch_mask = watch_get_mask(watch_list[i].events, watch_list[i].is_dir);

		/* is_dir == -1 means the wathing file/dir do not exist yet, dynamic add it */
		if (watch_list[i].is_dir == -1){

			/* Dynamic add watch */
			if (watch_dynamic_add(watch_fd, &watch_list[i]) == -1){

				fprintf(stderr, "Add [%s]:%s to inotify error:%s!\n", watch_list[i].name, watch_list[i].path, strerror(errno));
				continue;
			}
		}
		/* Add to inotify watch */	
		else{

			if ((watch_list[i].fd = inotify_add_watch(watch_fd, watch_list[i].path, watch_mask)) == -1){

				fprintf(stderr, "Add [%s]:%s to inotify error:%s!\n", watch_list[i].name, watch_list[i].path, strerror(errno));
				continue;
			}	
		}

	} /* end of for */
	
	/* Return inotify instance fd */
	return watch_fd;
}



/*************************************************************************************************************
**	@breif	:	check if the #event is watch watching event ?	
**	#watch	:	watch instance	
**	#event	:	current happend event
**	#msg	:	if match will fill this struct
**	@return	:	matched return 1, unmatch return 0
*************************************************************************************************************/
int watch_event_check(const P_WATCH_INFO watch, struct inotify_event *event, P_MSG_INFO msg)
{
	unsigned int i;
	const WT_EVENT *s_event;

	/* Check if it's match ? */	
	if (event->wd != watch->fd){

		return 0;
	}

	/* Set watch name to message */
	comm_set_msg_name(msg, watch->name);

	/* Check if event is a dir */
	comm_set_msg_dir(msg, event->mask);

	/* Set event happend path */
	if (watch->is_dir){

		comm_set_msg_path(msg, event->name);
	}
	else{

		comm_set_msg_path(msg, conf_basename(watch->path));
	}

	/* Check is speciall file add/del  */	
	if (watch->is_dir && watch->spc_file_cnt){

		for (i = 0; i < watch->spc_file_cnt; i++){
			
			if (strlen(watch->spc_file[i]) == strlen(event->name) && bcmp(watch->spc_file[i], event->name, strlen(event->name)) == 0){

				msg->spc[i] = 1;
				break;
			}
		}
	}
	
	/* Check evnets */
	for (s_event = &support_events[0]; s_event->name; s_event++){

		/* Nornal events is matched */
		if (event->mask & s_event->imask){

			msg->events	= s_event->mask;
			break;
		}
		/* Dir self-delete event speical process */
		else if (event->mask & IN_IGNORED && watch->is_dir && IS_SDEL_SET(watch->events)){

			msg->events	=	SDEL_MASK;
			comm_set_msg_dir(msg, IN_ISDIR);
			comm_set_msg_path(msg, conf_basename(watch->path));
			break;
		}
	}

	return msg->events;
}


/*************************************************************************************************************
**	@brief		:	watch dynamic event check
**	#watch_fd	:	inotify watch fd
**	#watch		:	watch 
**	#event		:	which event happend
**	@return		:	if it's dynamic event return 1, else 0
*************************************************************************************************************/
int watch_dynamic_check(int watch_fd, const P_WATCH_INFO watch, struct inotify_event *event)
{
	char *watching_file = conf_basename(watch->path);

	/* Check if it's match ? */	
	if (event->wd != watch->fd){

		return 0;
	}	

	/* It's not dynamic watch event */
	if (watch->dynamic_watch == 0){

		return 0;
	}

	/* Check if it's watching event */
	if (!(event->mask & IN_CREATE)){

		return 1;	
	}

	/* Check if it's watching file */
	if (strlen(event->name) == strlen(watching_file) && bcmp(event->name, watching_file, strlen(event->name)) == 0){

		/* It's dynamic watching events, delete dynamic watch  */	
		watch_dynamic_del(watch_fd, watch);
	}

	return 1;
}


