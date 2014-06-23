#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <strings.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/inotify.h>

#include "debug.h"
#include "comm.h"
#include "help.h"
#include "watch.h"
#include "configure.h"
#include "inotify_mod.h"


/* Global data */
volatile unsigned int debug = 0;
char *conf_file_path =	DEF_CONF_FILE;	


int main(int argc, char **argv)
{
	int i, j;
	MSG_INFO msg;
	int watch_num;
	int inotify_watch;
	dictionary	*conf;
	char events_buf[0x4000];
	P_WATCH_INFO watch_list;
	ssize_t r_size, offset;
	struct inotify_event *event;

	/* XXX:Debug and help options */
	if (argc >= 2 && argv[1][0] == '-'){
		
		print_help(argc, argv);		
	}


	/* Load ini file first */		
	if ((conf = iniparser_load(conf_file_path)) == NULL){
		
		fprintf(stderr, "Cann't parser file:%s\n", conf_file_path);
		return -1;
	}	

	/* Get watch numbers */	
	if ((watch_num = iniparser_getnsec(conf)) <= 0){
			
		fprintf(stderr, "Inotify_mod configure file format error!\n");
		goto out;
	}

	/* Malloc space for watchs */
	if ((watch_list = (P_WATCH_INFO)calloc( (sizeof(WATCH_INFO) * watch_num), 1)) == NULL){
	
		fprintf(stderr, "Malloc watch list error:%s!\n", strerror(errno));
		goto out;
	}

	/* Parser watch configure */
	if (conf_init(conf, watch_list, watch_num) != watch_num){

		fprintf(stderr, "Parser watch configure error!\n");
		goto out;
	}

	/* Inotify init */	
	if ((inotify_watch = watch_init(watch_list, watch_num)) == -1){

		fprintf(stderr, "Inotify watch instance init error!\n");
		goto out;
	}

	/* Comunication init */
	if (comm_init(conf)){
		
		fprintf(stderr, "Communication init failed!");
		goto out;
	}
	

	/* Wait events */
	while(1){


		/* Clear events buf and msg data */
		bzero(&msg, sizeof(msg));
		bzero(events_buf, sizeof(events_buf));
		

		/* If there have some events occurs, will read data structure */
		if ((r_size = read(inotify_watch, events_buf, sizeof(events_buf))) < 0) continue;

		offset = 0;
	
		/* Get events */
		event = (struct inotify_event *)events_buf;	

		/* Process each of the event */
		while(offset < r_size){

			
			/* Find which path has events occured */
			for (i = 0; i < watch_num; i++){

				/* Match ? */
				if (event->wd != watch_list[i].fd)
					continue;

				/* name */
				comm_set_msg_name(&msg, watch_list[i].name);

				/* Check if is dir */
				comm_set_msg_dir(&msg, event->mask);

				/* path */
				if (watch_list[i].is_dir){

					comm_set_msg_path(&msg, event->name);
				}
				else{

					comm_set_msg_path(&msg, basename(watch_list[i].path));
				}
			
				/* Watch idx */
				msg.idx = i;

				/* Check special file  */
				for (j = 0; watch_list[i].is_dir && watch_list[i].spc_name_cnt && j < MAX_SPC_FILE; j++){

					if (strlen(watch_list[i].spc_name[j]) == strlen(event->name)  &&   !bcmp(watch_list[i].spc_name[j], event->name, strlen(event->name))){

						msg.spc[j] = 1;
						break;
					}
				}

				/* add */
				if (event->mask & IN_CREATE){
					
					msg.events	=	ADD_MASK;
				}	

				/* del */
				if ((event->mask & IN_DELETE) || (event->mask & IN_DELETE_SELF)){

					msg.events	=	DEL_MASK;
				}
	
				/* mod */
				if ((event->mask  & IN_CLOSE_WRITE)){

					msg.events	=	MOD_MASK;
				}

				/* Send msg to remote or logger it */
				if (msg.events){

					comm_send_msg(&msg);
				}

			} /* end of for */

			
			/* Move to next */
			offset	+=	sizeof(struct inotify_event) + event->len;	

			event 	= (struct inotify_event *)(events_buf + offset);


		} /* end of while */
				

	} /* end of while */


	out:
	iniparser_freedict(conf);

	if (watch_list){

		free(watch_list);
		watch_list = NULL;
	}
	return 0;
}


