#ifndef __NETCONF_H
#define __NETCONF_H
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

extern struct netconn *order_netconn;	//全局TCP链接

/**
 * @type					:	在data_form.h中定义，结构体是req_type
 * @symbol					:	symbol是各类型的标志，也是在data_form.h中定义
 * @serverid_or_preservation: 	4字节数据
 * 								在订单状态中，高16位为批次序号，低16位为批次内序号；
 * 								在批次状态中，是批次序号(高16位)
 * 								在打印机状态中，是主控板打印单元序号，若无多个打印单元可以为0
 * 								在初次请求建立链接发送主控板时，是填充为0
 * 								在达到阈值请求数据报时，是填充为0
 */
#define NON_BASE_SEND_STATUS_ON 1//非必须反馈的状态
#if NON_BASE_SEND_STATUS_ON
	#define NON_BASE_SEND_STATUS(type, symbol, serverid_or_preservation) \
				write_connection(order_netconn, type, symbol, serverid_or_preservation)
#else
	#define NON_BASE_SEND_STATUS(type, symbol, serverid_or_preservation) 
#endif

#define BASE_SEND_STATUS_ON 1//必须反馈的状态
#if BASE_SEND_STATUS_ON
	#define BASE_SEND_STATUS(type, symbol, serverid_or_preservation) \
				write_connection(order_netconn, type, symbol, serverid_or_preservation)
#else
	#define BASE_SEND_STATUS(type, symbol, serverid_or_preservation) 
#endif


void LwIP_Init(void);
void lwip_demo(void);
void con_to_server(void);
void Recv_Batch(void);
//void Sent_Status(unsigned char order_No,unsigned char order_status);
void receive_connection(struct netconn *conn);
void put_in_buf(u8_t *data, u16_t len, u16_t urg);
/****************************************************************************************
*@Name............: write_connection
*@Description.....: 发送数据报
*@Parameters......: conn		:链接
*					type		:报文类型
*					symbol		:标志位
*					preservation:保留字段，批次中，高16位为批次序号，订单中时且低16位为批次内序号
*@Return values...: void
*****************************************************************************************/
void write_connection(struct netconn *conn, req_type type, u8_t symbol, u32_t preservation);
#endif
