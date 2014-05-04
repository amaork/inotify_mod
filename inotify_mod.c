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
#include "inotify_mod.h"
#include "iniparser/iniparser.h"
#include "socket/socket_cwrap.h"


/* XXX:Global data */
static unsigned int debug = 0;

static char *conf_file_path	=	DEF_CONF_FILE;	


/* XXX:Main */
int main(int argc, char **argv)
{
	int i, j;
	char *tmp;
	int inotify_watch;
	dictionary	*conf;
	unsigned int watch_num;
	P_WATCH_INFO watch_list;

	size_t r_size, offset;
	struct inotify_event *event;

	char msg_buf[0x100];
	char events_buf[0x4000];

	MSG_INFO msg;
	COMM_INFO comm;


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
	if (comm_init(conf, &comm)){
		
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
				bcopy(watch_list[i].name, msg.name, strlen(watch_list[i].name));	

				/* path */
				if (watch_list[i].is_dir){
					bcopy(event->name, msg.path, strlen(event->name));
				}
				else{

					tmp = basename(watch_list[i].path);
					bcopy(tmp, msg.path,  strlen(tmp));
				}
			
				/* Idx */
				msg.idx = i;

				/* special file  */
				for (j = 0; watch_list[i].is_dir && watch_list[i].spc_name_cnt && j < MAX_SPC_FILE; j++){

					if (strlen(watch_list[i].spc_name[j]) == strlen(event->name)  &&   !bcmp(watch_list[i].spc_name[j], event->name, strlen(event->name))){

						msg.spc[j] = 1;
						break;
					}
				}

				/* add */
				if (event->mask & IN_CREATE){
					
					msg.events	=	ADD_MASK;
					send_msg(&comm, &msg);	
					continue;
				}	

				/* del */
				if (event->mask & IN_DELETE){

					msg.events	=	DEL_MASK;
					send_msg(&comm, &msg);	
					continue;

				}
	
				/* mod */
				if ((event->mask  & IN_CLOSE_WRITE)  || (event->mask & IN_ATTRIB) ){

					msg.events	=	MOD_MASK;
					send_msg(&comm, &msg);	
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

/*************************************************
	@brief	Comunication init creat socket 
	@param	conf configure file dictionary
	@return	success return 0
**************************************************/
static int comm_init(dictionary *conf, P_COMM_INFO comm)
{
	char *addr;
	unsigned short port;
	unsigned int bcast;
	char bcast_addr[16];
	char port_str[8];


	/* Load comm configure */
	port	= 	iniparser_getint(conf, ":port", DEF_PORT);
	addr	=	iniparser_getstring(conf, ":addr", DEF_ADDR);
	comm->is_ascii = iniparser_getboolean(conf, ":ascii", DEF_ASCII);

	/* Debug */
	if (debug){
	
		fprintf(stderr, "Comm args:\n%s:%hd Ascii:%s\n", addr, port, comm->is_ascii ? "yes" : "no");
	}

	/* Check address if it was a broadcast addr or multicast addr */
	if (nw_is_bcast_addr(addr) == 0){

		if (debug)
			fprintf(stdout, "It's a broadcast address!\n");

		/* Enable broadcast send */
		if (nw_enable_broadcast()){

			fprintf(stdout, "Enable broadcast send error!\n");
			return -1;
		}
	}


	/* Check if it was multicast address then add to multicast gtroup */
	if (nw_is_mcast_addr(addr) == 0){

		if (debug)
			fprintf(stdout, "It's a multicast address!\n");
	}
	

	/* Fill COMM_INFO data structure */
	bzero(port_str, sizeof(port_str));
	snprintf(port_str, sizeof(port_str), "%hd", port);

	if (nw_get_sockaddr(&comm->addr, addr, port_str)){

		fprintf(stdout, "Get sockaddr error!\n");
		return -2;
	}

	/* Socket fd */
	comm->socket = nw_get_socket();

	
	return 0;
}


/*************************************************
	@brief	Load all watch's in ini file 
	@param	conf ini file dictionary 
	@param	watch parser ini configure file data
	@return	how many item in conf file 
*************************************************/
static int conf_init(dictionary *conf, P_WATCH_INFO watch, const unsigned int size)
{

	int i,j;	
	char *cp;
	char entry[32];
	struct stat fattr;
	unsigned int idx, nidx;

	/* Debug print all conf */
	if (debug)
		iniparser_dump(conf, stderr);
		

	/* Get section name's */
	for (i = 0; i < size; i++){

		cp = iniparser_getsecname(conf, i);
		bcopy(cp, watch[i].name, strlen(cp));
	} 

	/* Get each of them data */	
	for (i = 0; i < size; i++){

		/* Path */
		bzero(entry, sizeof(entry));
		snprintf(entry, sizeof(entry), "%s:%s", watch[i].name, PATH_KEY);

		cp = iniparser_getstring(conf, entry, "");
		bcopy(cp, watch[i].path, strlen(cp));	

		/* Ignore ? */
		bzero(entry, sizeof(entry));
		snprintf(entry, sizeof(entry), "%s:%s", watch[i].name, IGNORE_KEY);

		watch[i].ignore = iniparser_getboolean(conf, entry, 0);


		/* Comment */
		bzero(entry, sizeof(entry));
		snprintf(entry, sizeof(entry), "%s:%s", watch[i].name, COMMENT_KEY);

		cp = iniparser_getstring(conf, entry, "");
		bcopy(cp, watch[i].comment, strlen(cp));	

		/* Find if setting special file */
		bzero(entry, sizeof(entry));
		snprintf(entry, sizeof(entry), "%s:%s", watch[i].name, SPECIAL_KEY);

		if (iniparser_find_entry(conf, entry)){

			/* Read form ini file */
			cp  = iniparser_getstring(conf, entry, NULL);

			/* Parser special file */
			for (j = 0, idx = 0, watch[i].spc_name_cnt = 0; strlen(cp) && cp[0] == '[' &&  cp[j] != ']' && watch[i].spc_name_cnt < MAX_SPC_FILE; j++){

				switch (cp[j]){

					case	'['	:	
					case	']'	:	break;

					case	' '	:
					case	','	:
					case	'\t':	if (cp[j+1] != ' ' && cp[j+1] != ',' && cp[j+1] != '\t'){
								
										idx = 0;
										watch[i].spc_name_cnt ++;
									}
									break;

					default		:	if (debug){
	
										fprintf(stderr, "%c - %d - %d\n", cp[j], watch[i].spc_name_cnt, idx);
									}
								
									watch[i].spc_name[watch[i].spc_name_cnt][idx++] = cp[j];
									break;
	
				} /* end of switch */

			} /* end of for */

		} /* end of if */


		/* events */
		bzero(entry, sizeof(entry));
		snprintf(entry, sizeof(entry), "%s:%s", watch[i].name, EVENTS_KEY);
		
		cp	=	iniparser_getstring(conf, entry, "");

		/* Parser events */
		for (j = 0; strlen(cp) && cp[0] == '[' &&  cp[j] != ']'; j++){

			switch (cp[j]){

				case	'a'	:	if (cp[j + 1] == 'd' && cp[j + 2] == 'd'){
												
									watch[i].events |= 	ADD_MASK;
									j += 2;	
								}break;

				case	'd'	:	if (cp[j + 1] == 'e' && cp[j + 2] == 'l'){
										
									watch[i].events	|=	DEL_MASK;

									j += 2;	
								}break;

				case	'm'	:	if (cp[j + 1] == 'o' && cp[j + 2] == 'd'){

									watch[i].events	|=	MOD_MASK;
									j += 2;	
								}break;

			} /* end of switch */
	
		} /* end of for */


		/* Check if path is dir set is_dir mark */
		bzero(&fattr, sizeof(fattr));		

		if (!stat(watch[i].path, &fattr)){
		
			/* Test if the path is a dir */
			if (S_ISDIR(fattr.st_mode)){

				watch[i].is_dir = 1;
			}	
			else{
			
				watch[i].is_dir = 0;
			}
		}

		/* Debug */
		if (debug){
			fprintf(stderr, "\n[%s]\npath\t=\t%s\ndir\t=\t%s\nignore\t=\t%s\nevents\t=\t%s[%d]\ncomment\t=\t%s\n", \
					watch[i].name,  watch[i].path, watch[i].is_dir ? "yes" : "no", watch[i].ignore ? "yes" : "no", cp,  watch[i].events, watch[i].comment);	

	
			/* Print special file info */		
			if (watch[i].spc_name_cnt){

				for (j = 0; j <= watch[i].spc_name_cnt; j++){

					fprintf(stderr, "spe[%d]\t=\t%s\n", j, watch[i].spc_name[j]);
				}
			}

			fprintf(stderr, "\n\n\n");

		}	/* end if  */
	
	} /* end for */

	return (i == size) ? size : i;
}

/************************************************
	@brief	Create a inotify instance then add 
			watch_list path to the instace 
	@param	watch_list user conf data structure 
	@size	watch_list size 
	@return success return inotify instace fd
			falied return -1
************************************************/
static int watch_init(P_WATCH_INFO watch_list, unsigned int size)
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


/************************************************
	@brief	send events message
	@param	#comm communication data structure
	@param	#msg  will sending event message
	@return	success return 0, else return -1
************************************************/
static int send_msg(P_COMM_INFO comm, P_MSG_INFO msg)
{
	int len;
	char msg_buf[128];
	char *event_name;
	bzero(msg_buf, sizeof(msg_buf));	


	/* Convert event name */
	if (IS_ADD_SET(msg->events)){
		
		event_name = ADD_EVENT;	
	}
	else if (IS_DEL_SET(msg->events)){

		event_name = DEL_EVENT;
	}
	else if (IS_MOD_SET(msg->events)){

		event_name = MOD_EVENT;
	}

	/* Format ascii message */	
	snprintf(msg_buf, sizeof(msg_buf), "%d:%s:%s:%d%d%d%d%d%d%d%d:%s\r\n", \
			msg->idx, msg->name, event_name, msg->spc[0], msg->spc[1], msg->spc[2], msg->spc[3], msg->spc[4], msg->spc[5], msg->spc[6], msg->spc[7],  msg->path);	

	/* Debug */
	if (debug){
		fprintf(stdout, msg_buf);
	}


	/* XXX:Ascii format */
	if (comm->is_ascii){

		/* Send message to target */
		len = strlen(msg_buf);

		if (sendto(comm->socket, msg_buf, len, 0, (struct sockaddr*)&comm->addr, sizeof(comm->addr)) != len){

			perror("Sendto ascii:");
			return -1;
		}

		return 0;
	}

	/* XXX:Binary format */
	/* Convert to network order */
	msg->idx	=	htonl(msg->idx);
	msg->events	=	htonl(msg->events);

	len = sizeof(MSG_INFO); 

	if (sendto(comm->socket, msg, len, 0, (struct sockaddr*)&comm->addr, sizeof(comm->addr)) != len){

		perror("Sendto binary:");
		return -1;
	}


	return 0;
}


/************************************************
	@brief	print help message 
	@param	#argc how many arguments it have
	@param	#argv the arguments data 
	@return void 
************************************************/
static void print_help(int argc, char **argv)
{

	int i, j;

	for (i = 1; i < argc && argv[i][0] == '-'; i++){
	
		switch (argv[i][1]){

			/* Enable debug */
			case	'd'	:	debug = 1;break;

			/* Print format */
			case	'c'	:	fprintf(stdout, "%s\nConfigure file format using ini format: key = value;';' as comment\n", argv[0]);
							fprintf(stdout, "\nExample:\n");
							
							fprintf(stdout, "port\t=\tcommunication port\n");	
							fprintf(stdout, "addr\t=\ttarget address can be ucast,bcast,mcast addr\n");	
							fprintf(stdout, "ascii\t=\tyes -- message format is ascii, no -- binary\n");
							fprintf(stdout, "\n\n[NAME]\n");	
							fprintf(stdout, "path\t=\twatch directory or file abs path\n");	
							fprintf(stdout, "events\t=\t[add, del, mod]\t\t\t\t\t;One of them or combination\n");
							fprintf(stdout, "special\t=\t[file_name1, file_name2, ......]\t\t;Max support 8 file, only works with path is a directory\n");
							fprintf(stdout, "ignore\t=\tyes -- do not watch this conf\t\t\t;Default is no, do not need set\n");
							fprintf(stdout, "comment\t=\t\"The string at here is using for comment\"\t;The message at here just for easy read\n");
							exit(1);
				

			/* Load configure file */
			case	'f'	:	if (argc > (i+1)){

								conf_file_path	=	argv[i + 1];
							}	
							else{
	
								fprintf(stdout, "Please specify a configure file path after -f, such as %s -f test.ini\nDefault configure file:inotify_mod.ini\n", argv[0]);
								exit(1);
							}break;
		
			/* Print help */
			default 	:	
			case	'h'	:	fprintf(stdout, "%s\n\t-h\tPrint this message\n\t-d\tEnable debug message output\n\t-c\tPrint configure file format example\n\t-f\tSpecify alternate inotify_mod.ini configure file, default configure file:inotify_mod.ini\n", argv[0]);
							exit(0);
		}	
	}
}

