/**************************************************************
**	FileName:debug.h
**	Date: Mar 17 2010 Ver 0.1
**	Author: amaork+work@gmail.com
**	Description: 打印调试信息调试语句
**
**************************************************************/

#ifndef _DEBUG_H_
#define _DEBUG_H_

#define	_DEBUG_	1

/* If define _DEBUG_ then will print error message to screen */

#ifdef _DEBUG_

	/* Show which file and line message */
	#define __DEBUG_MESG__(mesg)			do { fprintf(stderr, "%s[%s] --> [%d] --> %s\n", __FILE__,__func__, __LINE__, mesg);} while(0)
	#define __debug_mesg__(format, args...)	fprintf(stderr, format, args)
#else

	#define __DEBUG_MESG__(mesg)			do {} while(0)
	#define __debug_mesg__(format, args...)	do {} while(0) 

#endif

#endif

