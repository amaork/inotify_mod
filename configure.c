#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
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
	unsigned int idx, i, j;

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
