#ifndef  __NTC_3950_H__
#define  __NTC_3950_H__


#define  NTC_R_T_MAP_MAX_IDX       80
#define  NTC_R_T_MAP_ERR_IDX       0xFF
#define  NTC_ERROR_T_VALUE         0x7FFF

#define  NTC_RBIAS_VALUE           10000 //欧姆

/*单倍温度值*/
int16_t ntc_3950_get_t(uint16_t adc);
/*10倍温度值 便于小数显示*/
int16_t ntc_3950_get_10x_t(uint16_t adc);




#endif