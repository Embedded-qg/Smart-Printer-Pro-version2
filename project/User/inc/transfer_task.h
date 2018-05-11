/***************************************************************************************
 *	File Name				: transfer_task.h
 *	CopyRight				: JockJo
 *	ModuleName			:	transfer_task
 *
 *	CPU						: 
 *	RTOS					:
 *
 *	Create Data				:	2017-12-09
 *	Author/Corportation		: JockJo
 *
 *	Abstract Description	:	transfer_task
 *
 *--------------------------------Revision History--------------------------------------
 *	No	version		Data			Revised By			Item			Description
 *	1    1.0      2017-12-09    JockJo   
 *
 ***************************************************************************************/


/**************************************************************
*	Multi-Include-Prevent Section
**************************************************************/
#ifndef __TRANSFERTASK_H
#define __TRANSFERTASK_H


/**************************************************************
*	Debug switch Section
**************************************************************/


/**************************************************************
*	Include File Section
**************************************************************/
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "pack_data.h"
#include "more_infomation.h"
#include "data_form.h"



/**************************************************************
*	Macro Define Section
**************************************************************/

/**************************************************************
*	Struct Define Section
**************************************************************/


/**************************************************************
*	Prototype Declare Section
**************************************************************/
u32_t Get_TARGET_ID(void);


void transf_task(struct netconn *conn, req_type type, u8_t symbol, u32_t target_id, u32_t preservation);


/**************************************************************
*	End-Multi-Include-Prevent Section
**************************************Tpro************************/
#endif  //__TRANSFERTASK_H
