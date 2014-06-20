#include "comm.h"
#include "socket/socket_cwrap.h"

/*************************************************************************************************************
**	@brief	:	Comunication init creat socket 
**	#conf	:	configure file dictionary
**	@return	:	success return 0, failed return -1
*************************************************************************************************************/
int comm_init(dictionary *conf, P_COMM_INFO comm)
{
	char *addr;
	char port_str[8];
	unsigned short port;


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

/*************************************************************************************************************
**	@brief	:	send events message
**	#comm	:	communication data structure
**	#msg  	:	will sending event message
**	@return	:	success return 0, else return -1
*************************************************************************************************************/
int comm_send_msg(P_COMM_INFO comm, P_MSG_INFO msg)
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


