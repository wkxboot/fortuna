#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "ntc_3950.h"
#define APP_LOG_MODULE_NAME   "[NTC]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_ERROR    
#include "app_log.h"

static const int32_t r_t_map[81][2]={
{153610,-30},{145516,-29},{137858,-28},{130614,-27},{123761,-26},{117280,-25},{111149,-24},{105351,-23},{99867 ,-22},{94681 ,-21},
{89776 ,-20},{85137 ,-19},{80750 ,-18},{76600 ,-17},{72676 ,-16},{68963 ,-15},{65451 ,-14},{62129 ,-13},{58986 ,-12},{56012 ,-11},
{53198 ,-10},{50534 , -9},{48013 , -8},{45627 , -7},{43368 , -6},{41229 , -5},{39204 , -4},{37285 , -3},{35468 , -2},{33747 , -1},
{32116 ,  0},{30570 ,  1},{29105 ,  2},{27716 ,  3},{26399 ,  4},{25150 ,  5},{23965 ,  6},{22842 ,  7},{21776 ,  8},{20764 ,  9},
{19783 , 10},{18892 , 11},{18026 , 12},{17204 , 13},{16423 , 14},{15681 , 15},{14976 , 16},{14306 , 17},{13669 , 18},{13063 , 19},
{12487 , 20},{11939 , 21},{11418 , 22},{10921 , 23},{10449 , 24},{10000 , 25},{9571  , 26},{9164  , 27},{8775  , 28},{8405  , 29},
{8052  , 30},{7716  , 31},{7396  , 32},{7090  , 33},{6798  , 34},{6520  , 35},{6255  , 36},{6002  , 37},{5760  , 38},{5529  , 39},
{5309  , 40},{5098  , 41},{4897  , 42},{4704  , 43},{4521  , 44},{4345  , 45},{4177  , 46},{4016  , 47},{3863  , 48},{3716  , 49},
{3588  , 50}
};

static uint8_t ntc_3950_seek_idex(uint32_t r)
{
 uint8_t mid=0;
 int low = 0;  
 int high =NTC_R_T_MAP_MAX_IDX;  
 if(r <=3588 || r >= 153610)//-29 ~49摄氏度
 {
   APP_LOG_ERROR("NTC 阻值超过范围！r=%d\r\n",r); 
   return NTC_R_T_MAP_ERR_IDX;
 }
 
  while (low <= high) 
  {  
  mid = (low + high) / 2;  
  if (r > r_t_map[mid][0])
  {
    if(r <= r_t_map[mid-1][0])
    return mid - 1;
    else
    high = mid - 1;  
  }
  else 
  {
   if(r > r_t_map[mid+1][0])
   return mid;
   else
   low = mid;    
  }  
 }
 return NTC_R_T_MAP_ERR_IDX; 
}
static uint32_t ntc_3950_get_r(uint16_t adc)
{
  uint16_t adc_in;
  uint32_t r;
  adc_in=adc;
  if(adc_in >= 4095)
    adc_in=4095;
  r=NTC_RBIAS_VALUE*adc_in/(4096-adc_in);
  
  return r;
}
/*单倍温度值 便于小数显示*/
int16_t ntc_3950_get_t(uint16_t adc)
{
  uint32_t r; 
  uint8_t idx;
  r=ntc_3950_get_r(adc);
  idx=ntc_3950_seek_idex(r);
  if(idx==NTC_R_T_MAP_ERR_IDX)
  return NTC_ERROR_T_VALUE;
  
  return r_t_map[idx][1];
}

/*10倍温度值 便于小数显示*/
int16_t ntc_3950_get_10x_t(uint16_t adc)
{
  uint32_t r,delta_r; 
  uint8_t idx;
  r=ntc_3950_get_r(adc);
  idx=ntc_3950_seek_idex(r);
  if(idx==NTC_R_T_MAP_ERR_IDX)
  return NTC_ERROR_T_VALUE;
  
  delta_r=r_t_map[idx][0]-r_t_map[idx+1][0]; 
  return 10*r_t_map[idx][1]+10*(r_t_map[idx][0]-r)/delta_r ;  
}
