#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__


//==========================================================
// <e> APP_LOG_ENABLED - app_log - Logging
//==========================================================
#ifndef APP_LOG_ENABLED
#define APP_LOG_ENABLED              1
#endif


#if  APP_LOG_ENABLED > 0
//***********************************************************/
#ifndef APP_LOG_USE_SEGGER_RTT
#define APP_LOG_USE_SEGGER_RTT       1
#endif

#ifndef APP_LOG_USE_UART
#define APP_LOG_USE_UART             0
#endif


#ifndef APP_LOG_USE_TIMESTAMP  
#define APP_LOG_USE_TIMESTAMP        1
#endif
                   
// <e> APP_LOG_USE_COLORS - If enabled then ANSI escape code for colors is prefixed to every string
//==========================================================
#ifndef APP_LOG_USE_COLORS
#define APP_LOG_USE_COLORS           1
#endif

// <o> APP_LOG_COLOR_DEFAULT  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef APP_LOG_COLOR_DEFAULT
#define APP_LOG_COLOR_DEFAULT         7
#endif

#ifndef APP_LOG_ERROR_COLOR
#define APP_LOG_ERROR_COLOR           2
#endif

#ifndef APP_LOG_WARNING_COLOR
#define APP_LOG_WARNING_COLOR         6
#endif

#ifndef APP_LOG_INFO_COLOR
#define APP_LOG_INFO_COLOR            3
#endif

#ifndef APP_LOG_DEBUG_COLOR
#define APP_LOG_DEBUG_COLOR           5
#endif


// <o> APP_LOG_DEFAULT_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef APP_LOG_DEFAULT_LEVEL
#define APP_LOG_DEFAULT_LEVEL         4
#endif

#ifndef APP_LOG_MODULE_NAME  
#define APP_LOG_MODULE_NAME          "[default]"
#endif

#ifndef APP_LOG_MODULE_LEVEL  
#define APP_LOG_MODULE_LEVEL          APP_LOG_DEFAULT_LEVEL
#endif

#endif //APP_LOG_ENABLED
#endif //__APP_LOG_CONFIG_H__

