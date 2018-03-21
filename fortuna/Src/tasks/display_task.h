#ifndef  __DISPLAY_TASK_H__
#define  __DISPLAY_TASK_H__

void display_task(void const * argument);
extern osThreadId display_task_hdl;


/*显示任务的数字buff*/
typedef struct 
{
 uint8_t        num;
 app_bool_t dp;
}dis_num_t;


/*显示任务的数字buff指针*/
extern const dis_num_t *ptr_display_buff;

#define  DISPLAY_TASK_FPS                        50/*每秒刷新值设置为50Hz*/
#define  DISPLAY_TASK_REFRESH_INTERVAL          (1000/DISPLAY_TASK_FPS)/*每次刷新间隔时间*/       
#define  DISPLAY_TASK_HOLD_ON_TIME_PER_POS       8






#endif