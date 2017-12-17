/***************************************************************************************
 *	File Name				: print_error.h
 *	CopyRight				: JockJo
 *	ModuleName			:	print_error
 *
 *	CPU						: 
 *	RTOS					:
 *
 *	Create Data				:	2017-12-17
 *	Author/Corportation		: JockJo
 *
 *	Abstract Description	:	print error
 *
 *--------------------------------Revision History--------------------------------------
 *	No	version		Data			Revised By			Item			Description
 *	1    1.0      2017-12-17    JockJo   
 *
 ***************************************************************************************/


/**************************************************************
*	Multi-Include-Prevent Section
**************************************************************/
#ifndef __PRINTERROR_H
#define __PRINTERROR_H


/**************************************************************
*	Debug switch Section
**************************************************************/


/**************************************************************
*	Include File Section
**************************************************************/
#include <stdio.h>


/**************************************************************
*	Macro Define Section
**************************************************************/
//DHCP  informations
#define ERROR_DHCP_START_FAILED  							"\nDHCP start failed!"
#define	ERROR_DHCP_IS_NOT_BE_CHOOSED					"\nDHCP is not  be choosed!\n"
#define	DHCP_CAN_BE_CHOOSED  				 					"\nDHCP can be choosed!"


//DM9161  informatiions
#define ERROR_DM9161_NETIF_IS_NULL 						"\nDM9161 netif's is null!"

//connection informations
#define ERROR_ORDER_CONNECTION_FAIL_TO_BUILD	"\nOrder Connection fail to build."
#define ORDER_CONNECTION_BUILD								"\nOrder Connection build."

//other informations
#define STR_IPADDR_IS													"\nstr_ipaddr is:"
#define ERROR_STR_IPADDR_IS_NULL							"\nstr_ipaddr is null!"

//printer information
#define ERROR_PRINTER_ALL_ERROR								"\nall of printer can not work!"
/**************************************************************
*	Struct Define Section
**************************************************************/


/**************************************************************
*	Prototype Declare Section
**************************************************************/


/**************************************************************
*	End-Multi-Include-Prevent Section
**************************************Tpro************************/
#endif  //__PRNTERROR_H
