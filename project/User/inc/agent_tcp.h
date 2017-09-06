/***************************************************************************************
 *	File Name				: agent_tcp.h
 *	CopyRight				: JockJo
 *	ModuleName			:	agent_tcp
 *
 *	CPU						: 
 *	RTOS					:
 *
 *	Create Data				:	2017-8-19
 *	Author/Corportation		: JockJo
 *
 *	Abstract Description	:	agent_tcp
 *
 *--------------------------------Revision History--------------------------------------
 *	No	version		Data			Revised By			Item			Description
 *	1    1.0      2017-8-19    JockJo   
 *
 ***************************************************************************************/


/**************************************************************
*	Multi-Include-Prevent Section
**************************************************************/
#ifndef __AGENTTCP_H
#define __AGENTTCP_H


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
#include "agent_udp.h"

/**************************************************************
*	Macro Define Section
**************************************************************/

/**************************************************************
*	Struct Define Section
**************************************************************/


/**************************************************************
*	Prototype Declare Section
**************************************************************/
void con_to_agent(void);  //与对等的agent建立TCP连接

void agent_tcp_server(void);	//本地服务器，监听peer连接

void send_to_peer(void);	 //向对等的ageng发送信息

void rec_from_peer(struct netconn *agent_newconn);		//从对等的agent接收数据

/**************************************************************
*	End-Multi-Include-Prevent Section
**************************************Tpro************************/
#endif  //__AGENTTCP_H
