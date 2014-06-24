#include "watch.h"
#include "events.h"

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

		/* Clear watch mask */
		watch_mask = 0;

		/* When ignor set yes, don't add to watch */
		if (watch_list[i].ignore)
			continue;		

		/* Make sure there's have some events */
		if (watch_list[i].events == 0){

			fprintf(stderr, "Watch[%s:%s] do not have valid events, won't add to watching list!\n", watch_list[i].name, watch_list[i].path);
			continue;
		}

		/* Get watch event and set watch mask */

		#if 1

		/* Add event, only support dir */
		if (IS_ADD_SET(watch_list[i].events)){

			watch_mask 	|=	IN_CREATE;
		}

		/* Delete event */
		if (IS_DEL_SET(watch_list[i].events)){

				watch_mask	|=	IN_DELETE;
		}

		/* Modify event */
		if (IS_MOD_SET(watch_list[i].events)){

			/* Dir */
			if (watch_list[i].is_dir){

				watch_mask |=	IN_CREATE | IN_DELETE;
			}
			/* File */
			else{
			
				watch_mask	|=	IN_CLOSE_WRITE;
			}
		}

		/* Read event */
		if (IS_READ_SET(watch_list[i].events)){


			watch_mask	|=	IN_ACCESS;
		}
		

		/* Self delete event */
		if (IS_SDEL_SET(watch_list[i].events)){

			/* Dir */
			if (watch_list[i].is_dir){

				watch_mask 	|=	IN_IGNORED;
			}
			/* File */
			else{
				
				watch_mask	|=	IN_DELETE_SELF;
			}
		}
		#endif
//		watch_mask = IN_ALL_EVENTS;


		/* Add to inotify watch */	
		if ((watch_list[i].fd = inotify_add_watch(watch_fd, watch_list[i].path, watch_mask)) == -1){

			fprintf(stderr, "Add [%s]:%s to inotify error:%s!\n", watch_list[i].name, watch_list[i].path, strerror(errno));
			continue;
		}	
	}
	
	/* Return inotify instance fd */
	return watch_fd;
}

