/**
  ******************************************************************************
  * File Name          : cbuff.c
  * Description        : ���������� ��� ������ � ���������� ��������
  *                      
  *                      
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "cbuff/cbuff.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void cbuff_init(cbuff_t* const cbuff, uint8_t* const mem, const uint16_t mem_size)
{
  /*
  если буфер слишком мал или его размер не является степенью двойки 
  */
  if(mem_size<4 || !((mem_size!=0) && !(mem_size&(mem_size-1))))
  {
    for(;;);
  }
  
  cbuff->buff=mem;
  cbuff->buff_size=mem_size;
  cbuff->buff_mask=mem_size-1;
  cbuff->in_idx=0;
  cbuff->out_idx=0;
}


/*
Запись в cbuff данных из src длинной len
*/
void write_to_cbuff(cbuff_t* const cbuff, uint8_t* const src, const uint16_t len)
{
  for(uint16_t i=0; i<len; i++)
  {
    cbuff->buff[cbuff->in_idx]=src[i];
    cbuff->in_idx++;
    cbuff->in_idx&=(cbuff->buff_mask);
  }
}
    
/*
Запись байта в cbuff
*/
void write_byte_to_cbuff(cbuff_t* const cbuff, const uint8_t src_byte)
{
    cbuff->buff[cbuff->in_idx]=src_byte;
    cbuff->in_idx++;
    cbuff->in_idx&=(cbuff->buff_mask);
}

/*
Принудительно установить указатель in_idx в cbuff, в случае если используется DMA для записи в cbuff
*/
void force_set_cbuff_in_idx(cbuff_t* const cbuff, const uint16_t dma_curr_data_pointer)
{
  cbuff->in_idx=(cbuff->buff_size-dma_curr_data_pointer)&((cbuff->buff_mask));
}

/*
Переместить указатель in_idx в cbuff, на размер принятых через DMA данных, если используется DMA для записи в cbuff
*/
void dma_offset_cbuff_in_idx(cbuff_t* const cbuff, const uint16_t dma_curr_data_pointer)
{
  cbuff->in_idx +=(dma_curr_data_pointer)&((cbuff->buff_mask));
}


/*
Чтение из cbuff данныx в dst длинной len
*/
void read_from_cbuff(cbuff_t* const cbuff, uint8_t* const dst, const uint16_t len)
{
  for(uint16_t i=0; i<len; i++)
  {
    dst[i]=cbuff->buff[cbuff->out_idx];
    cbuff->out_idx++;
    cbuff->out_idx&=(cbuff->buff_mask);
  }
}

/*
Чтение из cbuff данныx в dst длинной len без удаления
!Функция не тестировалась!
*/
void peek_from_cbuff(cbuff_t* const cbuff, uint8_t* const dst, const uint16_t len)
{
  uint16_t out_idx=cbuff->out_idx;
  
  for(uint16_t i=0; i<len; i++)
  {
    dst[i]=cbuff->buff[out_idx];
    out_idx++;
    out_idx&=(cbuff->buff_mask);
  }
}

/*
Чтение байта из cbuff
*/
uint8_t read_byte_from_cbuff(cbuff_t* const cbuff)
{
  uint8_t dst_byte;
  
  dst_byte=cbuff->buff[cbuff->out_idx];
  cbuff->out_idx++;
  cbuff->out_idx&=(cbuff->buff_mask);
  return dst_byte;
}

/*
Очистка len байт в cbuff читающим
*/
void clean_cbuff_len_from_reader(cbuff_t* const cbuff, const uint16_t len)
{
  cbuff->out_idx+=len;
  cbuff->out_idx&=(cbuff->buff_mask);
}

/*
Длина данных в cbuff 
*/
uint16_t filled_cbuff_len(cbuff_t* const cbuff)
{
  return ((cbuff->in_idx-cbuff->out_idx)&((cbuff->buff_mask)));
}

/*
Свободно в cbuff 
максимально в буфере может разместится cbuff->buff_size-1 байт, запись следующего приведёт к очистке буфера*/
uint16_t free_cbuff_len(cbuff_t* const cbuff)
{
  return ((cbuff->buff_mask)-((cbuff->in_idx-cbuff->out_idx)&((cbuff->buff_mask))));
}

/*
Очистка cbuff пишущим
*/
void clear_cbuff_from_writter(cbuff_t* const cbuff)
{
  cbuff->in_idx=(cbuff->out_idx)&((cbuff->buff_mask));
}

/*
Очистка cbuff читающим
*/
void clear_cbuff_from_reader(cbuff_t* const cbuff)
{
  cbuff->out_idx=(cbuff->in_idx)&((cbuff->buff_mask));
}

/*
Проверка, является ли буфер ref_buff длинной cmp_len концом кольцевого буфера cbuff?
!Функция не тестировалась!
*/
uint8_t is_cbuf_ends_with_ref_buff(cbuff_t* const cbuff, const uint8_t* const ref_buff, const uint16_t ref_len)
{
  if(ref_len<=filled_cbuff_len(cbuff))
  {
     for(uint16_t i=0, j=(cbuff->in_idx-ref_len)&(cbuff->buff_mask); i<ref_len; i++)
     {
       if(cbuff->buff[j]!=ref_buff[i]) return 0;
       j=(j+1)&((cbuff->buff_mask));
     }
     return 1;
  }
  else 
  {
      return 0;
  }
}

/*указатель на начало непрочитанных данных*/
uint8_t *cbuff_read_data(cbuff_t* const cbuff)
{
  uint16_t j = cbuff->out_idx;
  return &cbuff->buff[j];
}