/**
  ******************************************************************************
  * File Name          : cbuff.h
  * Description        : Библиотека для работы с кольцевыми буферами
  *                      
  *                      
  ******************************************************************************
*/

#ifndef __CBUFF_H
#define __CBUFF_H

#ifdef __cplusplus
extern "C" {
#endif 
  
  /* Includes -----------------------------------------------------------------*/  
  #include <stdint.h>
  
  /* Exported types -----------------------------------------------------------*/
  typedef struct
  {
    uint8_t* buff;
    uint16_t buff_size;
    uint16_t buff_mask;
    uint16_t in_idx;
    uint16_t out_idx;
  }cbuff_t;
  
  /* Exported constants -------------------------------------------------------*/ 
  /* Exported functions ------------------------------------------------------ */ 
  /*
  Инициализация cbuff
  */
  void cbuff_init(cbuff_t* const cbuff, uint8_t* const mem, const uint16_t mem_size);
    
  /*
  Запись в cbuff данных из src длинной len
  */
  void write_to_cbuff(cbuff_t* const cbuf, uint8_t* const src, const uint16_t len);
  
  /*
  Запись байта в cbuff
  */
  void write_byte_to_cbuff(cbuff_t* const cbuff, const uint8_t src_byte);
  
  /*
  Чтение из cbuff данныx в dst длинной len без удаления (не проверялась!!!)
  */
  void peek_from_cbuff(cbuff_t* const cbuff, uint8_t* const dst, const uint16_t len);
  
  /*
  Принудительно установить указатель in_idx в cbuff, в случае если используется DMA для записи в cbuff
  */
  void force_set_cbuff_in_idx(cbuff_t* const cbuff, const uint16_t dma_curr_data_pointer);
  
  /*
  Чтение из cbuff данныx в dst длинной len
  */
  void read_from_cbuff(cbuff_t* const cbuff, uint8_t* const dst, const uint16_t len);
  /*
  
  Чтение байта из cbuff
  */
  uint8_t read_byte_from_cbuff(cbuff_t* const cbuff);

  /*
  Очитска len байт в cbuff читающим
  */
  void clean_cbuff_len_from_reader(cbuff_t* const cbuff, const uint16_t len);
  
  /*
  Длина данных в cbuff 
  */
  uint16_t filled_cbuff_len(cbuff_t* const cbuff);
  
  /*
  Свободно в cbuff 
  */
  uint16_t free_cbuff_len(cbuff_t* const cbuff);
  /*
  
  Полная очистка пишущим
  */
  void clear_cbuff_from_writter(cbuff_t* const cbuff);
  
  /*
  Полная очистка cbuff читающим
  */
  void clear_cbuff_from_reader(cbuff_t* const cbuff);
    
  /*
  Проверка, является ли буфер ref_buff длинной cmp_len концом кольцевого буфера cbuff?
  !Функция не тестировалась!
  */
  uint8_t is_cbuf_ends_with_ref_buff(cbuff_t* const cbuff, const uint8_t* const ref_buff, const uint16_t cmp_len);
  
  /*
  указатель на начало непрочитанных данных
  */
  uint8_t *cbuff_read_data(cbuff_t* const cbuff);
  
#ifdef __cplusplus
}
#endif 

#endif /*__CBUFF_H*/