#ifndef __PORT_M_H__
#define __PORT_M_H__

#include <stdint.h>

#ifdef _cplusplus
extern          "C"
{
#endif

/* ----------------------- Defines ------------------------------------------*/

#define INLINE                         inline
#define STATIC                         static

#define PR_BEGIN_EXTERN_C              extern "C" {
#define	PR_END_EXTERN_C                }

#define MB_PORT_HAS_CLOSE              1
#define ENTER_CRITICAL_SECTION( )      vMBP_MASTER_PortEnterCritical( )
#define EXIT_CRITICAL_SECTION( )       vMBP_MASTER_PortExitCritical( )

#ifndef TRUE
#define TRUE                           ( BOOL )1
#endif

#ifndef FALSE
#define FALSE                          ( BOOL )0
#endif

/* ----------------------- Type definitions ---------------------------------*/

    typedef char    BOOL;

    typedef signed char BYTE;
    typedef unsigned char UBYTE;

    typedef unsigned char UCHAR;
    typedef char    CHAR;

    typedef unsigned short USHORT;
    typedef short   SHORT;

    typedef unsigned long ULONG;
    typedef long    LONG;

/* ----------------------- Function prototypes ------------------------------*/
    BOOL            bMBP_MASTER_IsWithinException( void );
    void            vMBP_MASTER_Init( void );
    void            vMBP_MASTER_PortEnterCritical( void );
    void            vMBP_MASTER_PortExitCritical( void );

#ifdef _cplusplus
}
#endif

#endif
