#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
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
unsigned int conf_parser_events(const char* events_str, unsigned int is_dir)
{
	int i, event_size; 
	const WT_EVENT *s_event;
	unsigned int events = 0;
	char event_name[MAX_EVENT_SIZE][32];

	/* Get events name form events string */
	bzero(event_name, sizeof(event_name));
	event_size = conf_get_words(events_str, event_name, MAX_EVENT_SIZE);

	/* Parser event name */
	for (i = 0; i < event_size; i++){

		//fprintf(stdout, "[%d]%s\n", i, event_name[i]);

		/* Check which event it want watch */	
		for (s_event = &support_events[0]; s_event->name; s_event++){

			/* Current watch is a file */
			if (!ev_is_support_event(is_dir, s_event->mask)){

				continue;
			}
			
			/* Check if it's support events */
			if (bcmp(event_name[i], s_event->name, strlen(event_name[i])) == 0){

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

		} /* end of for s_event */
	
	} /* end of for i */

	return events;
}


/*************************************************************************************************************
**	@brief	:	parser word form cp and save it to #words
**	#cp		:	will parser string pointer
**	#words	:	save words
**	#size	:	words array size
**	@return	:	return words size in cp 
*************************************************************************************************************/
int conf_get_words(const char *cp, char (*words)[32], unsigned int size)
{
	unsigned int i, j, idx, start, end;
	unsigned int str_size = strlen(cp);

	/* Nothing */
	if (str_size == 0){
	
		return 0;
	}

	/* String trip */
	for (i = 0; i < str_size; i++){

		if (!IS_WORD_END(cp[i])){

			start = i;
			break;
		}
	}

	for (i = str_size - 1;  i > start; i--){

		if (!IS_WORD_END(cp[i])){

			end = i;
			break;
		}
	}
	
	#if 0
	fprintf(stderr, "[%u]%s\n", str_size, cp);
	fprintf(stderr, "%u[%c]-%u[%c]\n", start, cp[start], end, cp[end]);
	#endif

	/* Parser string */	
	for (i = start, j = 0, idx = 0; i <= end && j < 32 && idx < size; i++){

		switch (cp[i]){
			
			/* Words end */
			case	' '	:
			case	','	:	
			case	'\t':
			case	'\n':	if (!IS_WORD_END(cp[i + 1])){
								
								/* Restart */
								j = 0;
								/* Move to next word */
								idx++;
							}break;

			/* Words */
			default		:	words[idx][j++]	= cp[i];	
							break;

		} /* end of switch */

	} /* end of for */

	return idx + 1;
}

/*************************************************************************************************************
**	@brief	:	safe swarp dirname
*************************************************************************************************************/
char *conf_dirname(const char *path)
{
	static char buffer[256];
	bzero(buffer, sizeof(buffer));
	bcopy(path, buffer, strlen(path));

	return dirname(buffer);
}

/*************************************************************************************************************
**	@brief	:	safe swarp basename
*************************************************************************************************************/
char *conf_basename(const char *path)
{
	static char buffer[256];
	bzero(buffer, sizeof(buffer));
	bcopy(path, buffer, strlen(path));

	return basename(buffer);
}

/*************************************************************************************************************
**	@brief	:	check if path is a dir	
**	#path	:	checking path
**	@return	:	#path is a dir return 1, not dir return 0, not exist return -1
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
		return -1;
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
					"ignore\t=\t%s\n", watch->name,  watch->path, watch->is_dir == 1 ? "yes" : "no", watch->ignore ? "yes" : "no");	

	fprintf(stdout, "events\t=\t[%s][%x]\n"
					"comment\t=\t%s\n", ev_get_events_str(watch->events, events_name_str, sizeof(events_name_str)), watch->events, watch->comment);

	
	/* Print special file info */		
	if (watch->spc_file_cnt){

		for (i = 0; i < watch->spc_file_cnt; i++){

				fprintf(stdout, "spe[%d]\t=\t%s\n", i, watch->spc_file[i]);
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
	unsigned int i;

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
			if ((cp  = iniparser_getstring(conf, entry, NULL))){
			
				/* Get special file name */
				watch[i].spc_file_cnt = conf_get_words(cp, watch[i].spc_file, MAX_SPC_FILE);
			}

		} 


		/* events */
		FORMAT_ENTRY(watch[i].name, EVENTS_KEY);
		GET_STR_ENTRY(watch[i].events_str, "");

		/* Parser events */
		watch[i].events	=	conf_parser_events(watch[i].events_str, watch[i].is_dir);

		/* Debug */
		if (debug){

			conf_print_watch(&watch[i]);
		}
	
	} /* end for */

	/* Arrange watching list */
	return conf_arrange_watch(watch, (i == size) ? size : i);
}
