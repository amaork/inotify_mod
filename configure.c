#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "watch.h"
#include "events.h"
#include "configure.h"


/*************************************************************************************************************
**	@brief	:	Load all watch's in ini file 
**	#conf	:	 ini file dictionary 
**	#watch 	:	parser ini configure file data
**	@return	:	how many item in conf file 
*************************************************************************************************************/
int conf_init(dictionary *conf, P_WATCH_INFO watch, const unsigned int size)
{

	char *cp;
	char entry[32];
	struct stat fattr;
	char events_name_str[128];
	unsigned int idx, i, j, event_idx;

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


		/* events */
		bzero(entry, sizeof(entry));
		snprintf(entry, sizeof(entry), "%s:%s", watch[i].name, EVENTS_KEY);
		
		cp	=	iniparser_getstring(conf, entry, "");

		/* Parser events */
		for (j = 0; strlen(cp) && cp[0] == '[' &&  cp[j] != ']'; j++){

			/* Check which event it want watch */	
			for (event_idx = 0; support_events[event_idx].name; event_idx++){

				/* Current watch is a file */
				if (!ev_is_support_event(watch[i].is_dir, support_events[event_idx].mask)){

					continue;
				}
			
				/* Check if it's support events */
				if (bcmp(cp + j, support_events[event_idx].name, strlen(support_events[event_idx].name)) == 0){

					/* Move to last name character  */
					j += strlen(support_events[event_idx].name) - 1;

					/* Make sure is full match test */
					if (cp[j + 1] == ' ' || cp[j + 1] == '\t' || cp[j + 1] == ',' || cp[j + 1] == ']'){

						watch[i].events	|=	support_events[event_idx].mask;
						break;
					}
				}
			}
	
		} /* end of for */

		/* Debug */
		if (debug){
			fprintf(stderr, "\n[%s]\npath\t=\t%s\ndir\t=\t%s\nignore\t=\t%s\n", watch[i].name,  watch[i].path, watch[i].is_dir ? "yes" : "no", watch[i].ignore ? "yes" : "no");	
			fprintf(stderr, "events\t=\t[%s][%x]\ncomment\t=\t%s\n", ev_get_events_str(watch[i].events, events_name_str, sizeof(events_name_str)), watch[i].events, watch[i].comment);

	
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
