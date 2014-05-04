#ifndef	_NETWORK_H_
#define	_NETWORK_H_

/* Get socket file descriptor */
extern int 	nw_get_socket(void);

/* Get system default ethernet interface name */
extern char* nw_get_def_iface(char *name, int size);

/* Set default  interface address as addr */
extern int  nw_set_ucast_addr(const char *addr);

/* Get system default interface broadcast addrerss */
extern int  nw_get_bcast_addr(unsigned int *netorder_addr);

/* Get system defualt interface unicast address */
extern int 	nw_get_ucast_addr(unsigned int *netorder_addr);

/* Check address is a broadcast addr or not */
extern int 	nw_is_bcast_addr(const char *addr);

/* Check address is multicast addr or not */
extern int 	nw_is_mcast_addr(const char *addr);

/* Enable socket broadcast option, it can send broadcast message */
extern int 	nw_enable_broadcast(void);

/* Enable reuse address */
extern int  nw_enable_reuseaddr(void);

/* Join to multicast group at default interface */
extern int 	nw_join_mcast_group(struct sockaddr_in group);

/* Leave a multicast group at defaunt interface */
extern int 	nw_leave_mcast_group(struct sockaddr_in group);

#endif
