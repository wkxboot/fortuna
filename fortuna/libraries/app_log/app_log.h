#ifndef   __APP_LOG_H__
#define   __APP_LOG_H__


#include "app_log_config.h"
#include "SEGGER_RTT.h"
#include "app_util.h"


#define APP_LOG_LEVEL_OFF          0U
#define APP_LOG_LEVEL_ERROR        1U
#define APP_LOG_LEVEL_WARNING      2U
#define APP_LOG_LEVEL_INFO         3U
#define APP_LOG_LEVEL_DEBUG        4U
#define APP_LOG_LEVEL_ARRAY        4U

#define APP_LOG_COLOR_CODE_DEFAULT "\x1B[0m"
#define APP_LOG_COLOR_CODE_BLACK   "\x1B[1;30m"
#define APP_LOG_COLOR_CODE_RED     "\x1B[1;31m"
#define APP_LOG_COLOR_CODE_GREEN   "\x1B[1;32m"
#define APP_LOG_COLOR_CODE_YELLOW  "\x1B[1;33m"
#define APP_LOG_COLOR_CODE_BLUE    "\x1B[1;34m"
#define APP_LOG_COLOR_CODE_MAGENTA "\x1B[1;35m"
#define APP_LOG_COLOR_CODE_CYAN    "\x1B[1;36m"
#define APP_LOG_COLOR_CODE_WHITE   "\x1B[1;37m"

#define APP_LOG_COLOR_0            APP_LOG_COLOR_CODE_DEFAULT
#define APP_LOG_COLOR_1            APP_LOG_COLOR_CODE_BLACK
#define APP_LOG_COLOR_2            APP_LOG_COLOR_CODE_RED
#define APP_LOG_COLOR_3            APP_LOG_COLOR_CODE_GREEN
#define APP_LOG_COLOR_4            APP_LOG_COLOR_CODE_YELLOW
#define APP_LOG_COLOR_5            APP_LOG_COLOR_CODE_BLUE
#define APP_LOG_COLOR_6            APP_LOG_COLOR_CODE_MAGENTA
#define APP_LOG_COLOR_7            APP_LOG_COLOR_CODE_CYAN
#define APP_LOG_COLOR_8            APP_LOG_COLOR_CODE_WHITE

#define APP_LOG_COLOR_DECODE(N)   CONCAT_2(APP_LOG_COLOR_, N)

#if APP_LOG_USE_COLORS  >  0
#define APP_LOG_ERROR_COLOR_CODE   APP_LOG_COLOR_DECODE(APP_LOG_ERROR_COLOR)
#define APP_LOG_WARNING_COLOR_CODE APP_LOG_COLOR_DECODE(APP_LOG_WARNING_COLOR)
#define APP_LOG_INFO_COLOR_CODE    APP_LOG_COLOR_DECODE(APP_LOG_INFO_COLOR)
#define APP_LOG_DEBUG_COLOR_CODE   APP_LOG_COLOR_DECODE(APP_LOG_DEBUG_COLOR)
#define APP_LOG_ARRAY_COLOR_CODE   APP_LOG_COLOR_DECODE(APP_LOG_ARRAY_COLOR)
#else 
#define APP_LOG_ERROR_COLOR_CODE
#define APP_LOG_WARNING_COLOR_CODE
#define APP_LOG_INFO_COLOR_CODE
#define APP_LOG_DEBUG_COLOR_CODE
#define APP_LOG_ARRAY_COLOR_CODE  
#endif 


#if APP_LOG_USE_TIMESTAMP  >  0
#define APP_LOG_TIMESTAMP_STRING    "[%8d]"
#define APP_LOG_TIMESTAMP_VALUE     APP_TIMESTAMP()
#else
#define APP_LOG_TIMESTAMP_STRING    "[%1d]" 
#define APP_LOG_TIMESTAMP_VALUE      0
#endif

#define APP_LOG_BREAK      ""

#define LOG_ERROR_PREFIX   APP_LOG_ERROR_COLOR_CODE   APP_LOG_TIMESTAMP_STRING APP_LOG_MODULE_NAME APP_LOG_BREAK "[ERROR]"
#define LOG_WARNING_PREFIX APP_LOG_WARNING_COLOR_CODE APP_LOG_TIMESTAMP_STRING APP_LOG_MODULE_NAME APP_LOG_BREAK "[WARNING]"
#define LOG_INFO_PREFIX    APP_LOG_INFO_COLOR_CODE    APP_LOG_TIMESTAMP_STRING APP_LOG_MODULE_NAME APP_LOG_BREAK "[INFO]"
#define LOG_DEBUG_PREFIX   APP_LOG_DEBUG_COLOR_CODE   APP_LOG_TIMESTAMP_STRING APP_LOG_MODULE_NAME APP_LOG_BREAK "[DEBUG]"
#define LOG_ARRAY_PREFIX   APP_LOG_ARRAY_COLOR_CODE   APP_LOG_TIMESTAMP_STRING APP_LOG_MODULE_NAME APP_LOG_BREAK "[ARRAY]"


#if APP_LOG_ENABLED > 0

#if APP_LOG_USE_SEGGER_RTT > 0
#define  APP_LOG_STD_OUT(format,arg...)     SEGGER_RTT_printf(0,format,APP_LOG_TIMESTAMP_VALUE,##arg)
#define  APP_LOG_INIT()                     SEGGER_RTT_Init() 

#elif APP_LOG_USE_UART > 0
#define  APP_LOG_STD_OUT(format,arg...)     printf(format,APP_LOG_TIMESTAMP_VALUE,##arg)
#define  APP_LOG_INIT()                     APP_LOG_UART_INIT()    

#else 
#error "APP_LOG_ENABLED为真，所以 APP_LOG_USE_UART 和 APP_LOG_USE_SEGGER_RTT 至少有一个为真! "//
#endif

#define APP_LOG_ERROR(format,arg...)                                                              \
    if ((APP_LOG_MODULE_LEVEL >= APP_LOG_LEVEL_ERROR) &&                                          \
        (APP_LOG_LEVEL_ERROR <= APP_LOG_DEFAULT_LEVEL))                                           \
    {                                                                                             \
      APP_LOG_STD_OUT(LOG_ERROR_PREFIX"line:%d\r\n"format,__LINE__,##arg);                        \
    }
    
#define APP_LOG_WARNING(format,arg...)                                                            \
    if ((APP_LOG_MODULE_LEVEL >= APP_LOG_LEVEL_WARNING) &&                                        \
        (APP_LOG_LEVEL_WARNING <= APP_LOG_DEFAULT_LEVEL))                                         \
    {                                                                                             \
      APP_LOG_STD_OUT(LOG_WARNING_PREFIX"line:%d\r\n"format,__LINE__,##arg);                      \
    }
    
#define APP_LOG_INFO(format,arg...)                                                               \
    if ((APP_LOG_MODULE_LEVEL >= APP_LOG_LEVEL_INFO) &&                                           \
        (APP_LOG_LEVEL_INFO <= APP_LOG_DEFAULT_LEVEL))                                            \
    {                                                                                             \
      APP_LOG_STD_OUT(LOG_INFO_PREFIX"line:%d\r\n"format,__LINE__,##arg);                         \
    }

#define APP_LOG_DEBUG(format,arg...)                                                              \
    if ((APP_LOG_MODULE_LEVEL >= APP_LOG_LEVEL_DEBUG) &&                                          \
        (APP_LOG_LEVEL_DEBUG <= APP_LOG_DEFAULT_LEVEL))                                           \
    {                                                                                             \
      APP_LOG_STD_OUT(LOG_DEBUG_PREFIX"line:%d\r\n"format,__LINE__,##arg);                        \
    }
    
#define APP_LOG_ARRAY(format,arg...)                                                              \
    if ((APP_LOG_MODULE_LEVEL >= APP_LOG_LEVEL_ARRAY) &&                                          \
        (APP_LOG_LEVEL_ARRAY <= APP_LOG_DEFAULT_LEVEL))                                           \
    {                                                                                             \
      APP_LOG_STD_OUT(LOG_ARRAY_PREFIX"\r\n"format,##arg);                        \
    }

#else
#define APP_LOG_ERROR(format,arg...)  
#define APP_LOG_WARNING(format,arg...)   
#define APP_LOG_INFO(format,arg...)  
#define APP_LOG_DEBUG(format,arg...) 
#endif


void app_log_init(void);
#if  APP_LOG_USE_UART > 0
void APP_LOG_UART_INIT(void);
#endif
#if APP_LOG_USE_TIMESTAMP > 0
uint32_t APP_TIMESTAMP(void);
#endif

#endif