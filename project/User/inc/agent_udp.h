/***************************************************************************************
 *	File Name				: agent_udp.h
 *	CopyRight				: JockJo
 *	ModuleName			:	agent_udp
 *
 *	CPU						: 
 *	RTOS					:
 *
 *	Create Data				:	2017-9-5
 *	Author/Corportation		: JockJo
 *
 *	Abstract Description	:	agent_udp
 *
 *--------------------------------Revision History--------------------------------------
 *	No	version		Data			Revised By			Item			Description
 *	1    1.0      2017-9-5    JockJo   
 *
 ***************************************************************************************/


/**************************************************************
*	Multi-Include-Prevent Section
**************************************************************/
#ifndef __AGENTUDP_H
#define __AGENTUDP_H


/**************************************************************
*	Debug switch Section
**************************************************************/


/**************************************************************
*	Include File Section
**************************************************************/
#include <stdio.h>
#include <string.h>
#include "lwip/opt.h"
#include "lwip/init.h"
#include "lwip/stats.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/pbuf.h"
#include "lwip/netif.h"
#include "lwip/sockets.h"
#include "lwip/ip.h"
#include "lwip/raw.h"
#include "lwip/udp.h"
#include "lwip/tcp_impl.h"
#include "lwip/snmp_msg.h"
#include "lwip/autoip.h"
#include "lwip/igmp.h"
#include "lwip/dns.h"
#include "lwip/timers.h"
#include "netif/etharp.h"
#include "lwip/api.h"
#include "tcpip.h"
#include "lwip/memp.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "ethernetif.h"
#include "data_form.h"
#include "queue_buf.h"
#include "main.h"
#include "pack_data.h"
#include "more_infomation.h"
#include "analyze_data.h"
#include "app_cfg.h"
#include "data_form.h"
#include "more_infomation.h"

/**************************************************************
*	Macro Define Section
**************************************************************/

/**************************************************************
*	Struct Define Section
**************************************************************/

/**************************************************************
*	Prototype Declare Section
**************************************************************/
void create_buf_pbuf(struct netbuf *newbuf, struct pbuf *newpbuf, int sizeOfNewbuf);
void add_sned_data(struct netbuf *buf, u8_t *data);
void multicast_to_localLAN(void);
static void set_ipaddr(void);
void init_client_server(void);
void init_UDP(void);


/**************************************************************
*	End-Multi-Include-Prevent Section
**************************************Tpro************************/
#endif  //__AGENTUDP_H
