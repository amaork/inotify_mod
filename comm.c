#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#include "comm.h"
#include "events.h"
#include "socket/socket_cwrap.h"

static COMM_INFO comm_info;

/*************************************************************************************************************
**	@brief	:	Comunication init creat socket 
**	#conf	:	configure file dictionary
**	@return	:	success return 0, failed return -1
*************************************************************************************************************/
int comm_init(dictionary *conf)
{
	char *addr;
	char port_str[8];
	unsigned short port;


	/* Load comm configure */
	port				= 	iniparser_getint(conf, ":port", DEF_PORT);
	addr				=	iniparser_getstring(conf, ":addr", DEF_ADDR);
	comm_info.is_ascii 	= 	iniparser_getboolean(conf, ":ascii", DEF_ASCII);

	/* Debug */
	if (debug){
	
		fprintf(stderr, "Comm args:\n%s:%hd Ascii:%s\n", addr, port, comm_info.is_ascii ? "yes" : "no");
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

	if (nw_get_sockaddr(&comm_info.addr, addr, port_str)){

		fprintf(stdout, "Get sockaddr error!\n");
		return -2;
	}

	/* Socket fd */
	comm_info.socket = nw_get_socket();

	
	return 0;
}

/*************************************************************************************************************
**	@brief	:	send events message
**	#msg  	:	will sending event message
**	@return	:	success return 0, else return -1
*************************************************************************************************************/
int comm_send_msg(P_MSG_INFO msg)
{
	int len, idx;
	char msg_buf[128];
	char *event_name;
	bzero(msg_buf, sizeof(msg_buf));	


	/* Convert event name */
	for (idx = 0; support_events[idx].name; idx++){

		if (msg->events & support_events[idx].mask){

			event_name = support_events[idx].name;
			break;
		}
	}

	/* Format ascii message */	
	snprintf(msg_buf, sizeof(msg_buf), "%d:%s:%s:%d%d%d%d%d%d%d%d:%s:%d\r\n", \
			msg->idx, msg->name, event_name, msg->spc[0], msg->spc[1], msg->spc[2], msg->spc[3], msg->spc[4], msg->spc[5], msg->spc[6], msg->spc[7],  msg->path, msg->is_dir);	

	/* Debug */
	if (debug){
		fprintf(stdout, msg_buf);
	}


	/* XXX:Ascii format */
	if (comm_info.is_ascii){

		/* Send message to target */
		len = strlen(msg_buf);

		if (sendto(comm_info.socket, msg_buf, len, 0, (struct sockaddr*)&comm_info.addr, sizeof(comm_info.addr)) != len){

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

	if (sendto(comm_info.socket, msg, len, 0, (struct sockaddr*)&comm_info.addr, sizeof(comm_info.addr)) != len){

		perror("Sendto binary:");
		return -1;
	}


	return 0;
}

/*************************************************************************************************************
**	@brief	:	set MSG_INFO field
*************************************************************************************************************/
inline void comm_set_msg_name(P_MSG_INFO msg, char *name)
{
	bzero(msg->name, sizeof(msg->name));
	bcopy(name, msg->name, strlen(name));	
}

inline void comm_set_msg_path(P_MSG_INFO msg, char *path)
{
	bzero(msg->path, sizeof(msg->path));
	bcopy(path, msg->path, strlen(path));
}

inline void comm_set_msg_dir(P_MSG_INFO msg, unsigned int mask)
{
	/* Check if is dir */
	if (mask & IN_ISDIR){

		msg->is_dir = 1;
	}
	else{
		
		msg->is_dir = 0;
	}
}

