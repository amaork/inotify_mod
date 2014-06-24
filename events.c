#include "events.h"

const WT_EVENT support_events[] = {

	{ALL_EVENT,		ALL_MASK,	0},
	{ADD_EVENT,		ADD_MASK,	I_ADD_MASK},
	{DEL_EVENT,		DEL_MASK,	I_DEL_MASK},

	{MOD_EVENT,		MOD_MASK,	I_MOD_MASK},
	
	{READ_EVENT,	READ_MASK,	I_READ_MASK},
	{SDEL_EVENT,	SDEL_MASK,	I_SDEL_MASK},	
	{0},
};


/*************************************************************************************************************
**	@brief	:	convert events mask as readaible string
**	#events	:	watch events mask
**	#buf	:	save string buffer
**	#size	:	buf size
**	@return	:	return buf pointer
*************************************************************************************************************/
char *ev_get_events_str(unsigned int events, char *buf, int size)
{
	int idx;	
	bzero(buf, size);	

	/* Convert events mask to name */
	for (idx = 0; support_events[idx].name; idx++){

		if (events & support_events[idx].mask){

			snprintf(buf + strlen(buf), size, " %s", support_events[idx].name);
		}
	}

	/* Add a space at the end */
	buf[strlen(buf)] = ' ';

	return buf;
}

/*************************************************************************************************************
**	@brief	:	check watch support this event
**	#is_dir	:	watch is_dir falg
**	#event	:	watch event 
**	@retun	:	if support return 1, unsupport return 0
*************************************************************************************************************/
inline int ev_is_support_event(unsigned int is_dir, unsigned int  event)
{
	/* All events ? */
	if (IS_ALL_SET(event)){
		
		return 1;
	}

	/* Dir */
	if (is_dir){

		return IS_DIR_SUPPORT(event);
	}
	/* File */
	else{
	
		return IS_FILE_SUPPORT(event);
	}
}
