#include "help.h"

/*************************************************************************************************************
**	@brief	:	print help message 
**	#argc 	:	how many arguments it have
**	#argv t	:	he arguments data 
*************************************************************************************************************/
void print_help(int argc, char **argv)
{

	int i;

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
			case	'h'	:	fprintf(stdout, "%s\n"
											"\t-h\tPrint this message\n"
											"\t-d\tEnable debug message output\n"
											"\t-c\tPrint configure file format example\n"
											"\t-f\tSpecify alternate inotify_mod.ini configure file, default configure file:inotify_mod.ini\n", argv[0]);
							exit(0);
		}	
	}
}

