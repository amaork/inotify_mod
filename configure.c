#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "watch.h"
#include "events.h"
#include "configure.h"

/*************************************************************************************************************
**	@brief	:	Arrange watching list remove invalid events watch
**	#wlist	:	Watching list
**	#size	:	Watching list size
**	@return	:	return valid watch size in watching list
*************************************************************************************************************/
static int conf_arrange_watch(P_WATCH_INFO wlist, unsigned int size)
{
	unsigned int i, j, k, vsize;

	/* Remove invalid watch form wathing list */
	for (i = 0, vsize = 0; i < size;){

		/* Find invalid watch */
		if (wlist[i].events == 0 || wlist[i].ignore){

			/* Remove invalid watch */
			for (j = i, k = i + 1; k < size;){

				wlist[j++] = wlist[k++];
			}

			/* Remove last watch */
			size--;
		}
		else{
			
			/* Move to next */
			i++;	
			vsize++;
		}
	}

	fprintf(stderr, "Wating list watch size:%u\n", vsize);	

	return vsize;
}


/*************************************************************************************************************
**	@brief		:	parser events
**	#events_str	:	will parsered events string
**	#is_dir		:	is watch file is a dir
**	@return		:	return watch events mask 
*************************************************************************************************************/
static unsigned int conf_parser_events(const char* events_str, unsigned int is_dir)
{
	int i; 
	const WT_EVENT *s_event;
	unsigned int events = 0;

	/* Parser events */
	for (i = 0; strlen(events_str) && events_str[0] == '[' &&  events_str[i] != ']'; i++){

		/* Check which event it want watch */	
		for (s_event = &support_events[0]; s_event->name; s_event++){

			/* Current watch is a file */
			if (!ev_is_support_event(is_dir, s_event->mask)){

				continue;
			}
			
			/* Check if it's support events */
			if (bcmp(events_str + i, s_event->name, strlen(s_event->name)) == 0){

				/* Move to last name character  */
				i += strlen(s_event->name) - 1;

				/* Make sure is full match test */
				if (IS_WORD_END(events_str[i + 1])){

					/* All events */
					if (IS_ALL_SET(s_event->mask)){

						events |=	is_dir ? DIR_SUPPORT_EVENTS : FILE_SUPPORT_EVENTS;
						break;
					}
					/* Single events */
					else{

						events	|=	s_event->mask;
						break;
					}
				}

			}

		} /* end of for s_event */
	
	} /* end of for i */

	return events;
}

/*************************************************************************************************************
**	@brief	:	check if path is a dir	
**	#path	:	checking path
**	@return	:	#path is a dir return 1, not dir return 0
*************************************************************************************************************/
int conf_is_path_is_dir(char *path)
{
	struct stat fattr;
	bzero(&fattr, sizeof(fattr));		

	/* Get path stat */
	if (stat(path, &fattr) == 0){
		
		/* Test if the path is a dir */
		if (S_ISDIR(fattr.st_mode)){

			return 1;
		}	
		else{

			return 0;
			
		}
	}
	else{

		fprintf(stderr, "Get path[%s] stat failed:%s!\n", path, strerror(errno));
	}

	return 0;
}


/*************************************************************************************************************
**	@brief	:	print watch debug message
**	#watch	:	will print watch data structure
*************************************************************************************************************/
void conf_print_watch(const P_WATCH_INFO watch)
{
	unsigned int i;
	char events_name_str[128];

	/* Print */
	fprintf(stdout, "\n[%s]\n"
					"path\t=\t%s\n"
					"dir\t=\t%s\n"
					"ignore\t=\t%s\n", watch->name,  watch->path, watch->is_dir ? "yes" : "no", watch->ignore ? "yes" : "no");	

	fprintf(stdout, "events\t=\t[%s][%x]\n"
					"comment\t=\t%s\n", ev_get_events_str(watch->events, events_name_str, sizeof(events_name_str)), watch->events, watch->comment);

	
	/* Print special file info */		
	if (watch->spc_name_cnt){

		for (i = 0; i <= watch->spc_name_cnt; i++){

				fprintf(stdout, "spe[%d]\t=\t%s\n", i, watch->spc_name[i]);
			}
		}

	fprintf(stdout, "\n\n\n");
}


/*************************************************************************************************************
**	@brief	:	Load all watch's in ini file 
**	#conf	: 	ini file dictionary 
**	#watch 	:	parser ini configure file data
**	@return	:	how many item in conf file 
*************************************************************************************************************/
int conf_init(dictionary *conf, P_WATCH_INFO watch, const unsigned int size)
{

	char entry[64];
	char *cp = NULL;
	unsigned int idx, i, j;

	#define GET_INT_ENTRY(v, d)		do { v = iniparser_getint(conf, entry, (d)); } while(0)
	#define GET_BOOL_ENTRY(v, d)	do { v = iniparser_getboolean(conf, entry, (d)); } while(0)
	#define GET_STR_ENTRY(s, d)		do { cp  = iniparser_getstring(conf, entry, (d)); bcopy(cp, s, strlen(cp)); } while (0)
	#define FORMAT_ENTRY(s, k)		do { bzero(entry, sizeof(entry)); snprintf(entry, sizeof(entry), "%s:%s", (s), (k)); } while(0)

	/* Debug print all conf */
	if (debug){

		iniparser_dump(conf, stderr);
	}
		
	/* Get section name's */
	for (i = 0; i < size; i++){

		cp = iniparser_getsecname(conf, i);
		bcopy(cp, watch[i].name, strlen(cp));
	} 

	/* Get each of them data */	
	for (i = 0; i < size; i++){

		/* Ignore ? */
		FORMAT_ENTRY(watch[i].name, IGNORE_KEY);
		GET_BOOL_ENTRY(watch[i].ignore, 0);

		/* If ignore, do not read jump to next */
		if (watch[i].ignore){
			
			continue;	
		}

		/* Path */
		FORMAT_ENTRY(watch[i].name, PATH_KEY);
		GET_STR_ENTRY(watch[i].path, NULL);

		/* Check if path is dir set is_dir mark */
		watch[i].is_dir = conf_is_path_is_dir(watch[i].path);

		/* Comment */
		FORMAT_ENTRY(watch[i].name, COMMENT_KEY);
		GET_STR_ENTRY(watch[i].comment, NULL);

		/* Find if setting special file */
		FORMAT_ENTRY(watch[i].name, SPECIAL_KEY);
		
		/* Only dir have this option */
		if (watch[i].is_dir && iniparser_find_entry(conf, entry)){

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

					default		:	watch[i].spc_name[watch[i].spc_name_cnt][idx++] = cp[j];
									break;
	
				} /* end of switch */

			} /* end of for */

		} /* end of if */


		/* events */
		FORMAT_ENTRY(watch[i].name, EVENTS_KEY);
		cp	=	iniparser_getstring(conf, entry, "");

		/* Parser events */
		watch[i].events	=	conf_parser_events(cp, watch[i].is_dir);

		/* Debug */
		if (debug){

			conf_print_watch(&watch[i]);
		}

	
	} /* end for */

	/* Arrange watching list */
	return conf_arrange_watch(watch, (i == size) ? size : i);
}
