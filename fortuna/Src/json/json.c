#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "string.h"
#include "app_common.h"
#include "json.h"

static uint8_t json_body_str[JSON_BODY_STR_MAX_SIZE];

app_bool_t json_get_value_by_name_from_json_body(uint8_t *ptr_json,uint8_t *ptr_name,uint8_t *ptr_value)
{
  uint8_t *ptr_addr;
  uint8_t cnt=0;
  if(ptr_json==NULL)
  return APP_FALSE; 
  ptr_addr=(uint8_t *)strstr((const char *)ptr_json,(const char *)ptr_name);
  if(ptr_addr==NULL)
  return APP_FALSE;
  ptr_addr=(uint8_t *)strstr((const char *)ptr_addr,":");/*查找“：”*/
  if(ptr_addr==NULL)
  return APP_FALSE;
  ptr_addr++;
  while(*ptr_addr!=',' && *ptr_addr!='}')
  {
  ptr_value[cnt++]=*ptr_addr++; 
  }
  ptr_value[cnt]=0;/*设置成字符串*/
  return APP_TRUE;
}

app_bool_t json_find_json_body(uint8_t *ptr_buff,uint8_t **ptr_json)
{
  uint8_t *ptr_addr_start;
  app_bool_t result=APP_FALSE;
  ptr_addr_start=(uint8_t *)strstr((const char *)ptr_buff,"{");
  if(ptr_addr_start)
  {
   *ptr_json=ptr_addr_start;
   result= APP_TRUE;
  }
  return result;
}



app_bool_t json_set_item_name_value(json_item_t *ptr_item,uint8_t *ptr_name,uint8_t *ptr_value)
{
 if(ptr_item==NULL )
 {
  return APP_FALSE; 
 }
 if(ptr_name)
 strcpy((char *)ptr_item->name,(const char *)ptr_name);
 if(ptr_value)
 strcpy((char *)ptr_item->value,(const char *)ptr_value);
 
 return APP_TRUE; 
}

app_bool_t json_body_to_str(void *ptr_json,uint8_t **ptr_str)
{
 uint8_t len=0;
 uint8_t i,cnt;
 uint8_t *ptr_str_buff=json_body_str;
 json_item_t *ptr_item;
 if(ptr_json==NULL)
 return APP_FALSE;
 cnt=((json_header_t*)ptr_json)->item_cnt;
 
 ptr_item=(json_item_t*)((uint8_t *)ptr_json+sizeof(json_header_t));;
 *ptr_str_buff++='{';
 
  for(i=0;i<cnt;i++)
  {
  len=strlen((const char *)ptr_item->name);
  memcpy(ptr_str_buff,ptr_item->name,len);
  ptr_str_buff+=len;
  *ptr_str_buff++=':';
  len=strlen((const char *)ptr_item->value);
  memcpy(ptr_str_buff,ptr_item->value,len);
  ptr_str_buff+=len;
  *ptr_str_buff++=',';
  ptr_item++;
  }
 *(ptr_str_buff-1)='}';
 *ptr_str_buff=0;
 *ptr_str=json_body_str;
 return APP_TRUE; 
}


