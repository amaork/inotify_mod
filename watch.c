#include "watch.h"

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

		/* Get watch event and set watch mask */
		if (IS_ADD_SET(watch_list[i].events))
			watch_mask 	|=	IN_CREATE;

		if (IS_DEL_SET(watch_list[i].events))
			watch_mask	|=	IN_DELETE;

		if (IS_MOD_SET(watch_list[i].events))
			watch_mask	|=	IN_CLOSE_WRITE | IN_ATTRIB;


		/* Add to inotify watch */	
		if ((watch_list[i].fd = inotify_add_watch(watch_fd, watch_list[i].path, watch_mask)) == -1){

			fprintf(stderr, "Add [%s]:%s to inotify error:%s!\n", watch_list[i].name, watch_list[i].path, strerror(errno));
			continue;
		}	
	}
	
	/* Return inotify instance fd */
	return watch_fd;
}



