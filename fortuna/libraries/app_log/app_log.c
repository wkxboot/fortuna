#define APP_LOG_MODULE_NAME   "[app_log]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"


void app_log_init()
{
#if APP_LOG_ENABLED > 0  
 APP_LOG_INIT();
 APP_LOG_DEBUG("APP LOG INITED!\r\n");
#endif
}






