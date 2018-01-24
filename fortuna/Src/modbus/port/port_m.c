/*******************************************************************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * modify by wkxboot 2017.7.11 
 * *****************************************************************************
 */
/* ----------------------- System includes ----------------------------------*/
#include <stdlib.h>
/* ----------------------- Platform includes --------------------------------*/
#include "stm32f1xx.h"


/* ----------------------- Modbus includes ----------------------------------*/
#include "port_m.h"
#include "cmsis_os.h"

#define APP_LOG_MODULE_NAME   "[MB_OTHER]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_ERROR   
#include "app_log.h"
/* ----------------------- Defines ------------------------------------------*/

/* ----------------------- Type definitions ---------------------------------*/

/* ----------------------- Function prototypes ------------------------------*/
void            vMB_MASTER_PortSerialClose( void );
void            vMB_MASTER_PortEventClose( void );
void            vMB_MASTER_PortTimerClose( void );

/* ----------------------- Static variables ---------------------------------*/

/* ----------------------- Static functions ---------------------------------*/

/* ----------------------- Start implementation -----------------------------*/

void
vMB_MASTER_PInit( void )
{
  /*
    AT91F_AIC_Open( AT91C_BASE_AIC, AT91C_AIC_BRANCH_OPCODE, AT91C_AIC_BRANCH_OPCODE,
                    vDefaultHandler, vDefaultSpuriousHandler, AT91C_AIC_DCR_PROT );
*/
}

void
master__assert( const char *pcFile, const char *pcAssertion, int iLine )
{
    volatile BOOL   bBreakOut = FALSE;

    ( void )pcFile;
    ( void )pcAssertion;
    ( void )iLine;
    ENTER_CRITICAL_SECTION(  );
    while( !bBreakOut );
}

BOOL
bMBP_MASTER_IsWithinException( void )
{
   return (__get_IPSR()!=0) ;
}

void
vMBP_MASTER_PortEnterCritical( void )
{
    if( !bMBP_MASTER_IsWithinException(  ) )
    {
      APP_LOG_DEBUG("临界进.\r\n");
      portENTER_CRITICAL(  );
    }
}

void
vMBP_MASTER_PortExitCritical( void )
{
    if( !bMBP_MASTER_IsWithinException(  ) )
    {
     APP_LOG_DEBUG("临界出.\r\n");
     portEXIT_CRITICAL(  );
    }
}

void
vMB_MASTER_PortClose( void )
{
    vMB_MASTER_PortSerialClose(  );
    vMB_MASTER_PortEventClose(  );
    vMB_MASTER_PortTimerClose(  );
}
