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
#include "events.h"
#include "configure.h"
#include "inotify_mod.h"


/* Global data */
volatile unsigned int debug = 0;
char *conf_file_path =	DEF_CONF_FILE;	


int main(int argc, char **argv)
{
	int i;
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
	if ((watch_num = conf_init(conf, watch_list, watch_num)) <= 0){

		fprintf(stderr, "There's no invalid watch in watching list, exit!\n");
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
	while(watch_num){

		offset = 0;

		/* Clear events buf and msg data */
		bzero(events_buf, sizeof(events_buf));
		
		/* If there have some events occurs, will read data structure */
		if ((r_size = read(inotify_watch, events_buf, sizeof(events_buf))) < 0) continue;
	
		/* Get events */
		event = (struct inotify_event *)events_buf;	

		/* Process each of the event */
		while(offset < r_size){
			
			/* Find which path has events occured */
			for (i = 0; i < watch_num; i++){

				bzero(&msg, sizeof(msg));

				/* Dynamic add event check */
				if (watch_dynamic_check(inotify_watch, &watch_list[i], event)){

					continue;
				}

				/* Normal event check */
				if (watch_event_check(&watch_list[i], event, &msg)){

					msg.idx	=	i;
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


