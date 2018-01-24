#ifndef  __FORTUNA_COMMON_H__
#define  __FORTUNA_COMMON_H__
#include "stdint.h"

typedef enum
{
FORTUNA_TRUE =1,
FORTUNA_FALSE=0
}fortuna_bool_t;


/*门/锁/UPS的状态*/
typedef enum
{
 LOCK_LOCK_SUCCESS=1,
 LOCK_LOCK_FAIL=0,
 LOCK_UNLOCK_SUCCESS=1,
 LOCK_UNLOCK_FAIL=1,
 
 LOCK_LOCKED=0,
 LOCK_UNLOCKED=1,
 LOCK_LOCKING=2,
 LOCK_FAULT=0xFF,
 UPS_PWR_ON=1,
 UPS_PWR_OFF=2,
 UPS_FAULT=0xFF
}obj_status_t;

/*长虹id*/
#define  VENDOR_ID_CHANGHONG                  0x11
/*固件版本号v1.0*/
#define  FIRMWARE_VERSION                     0x01







#endif