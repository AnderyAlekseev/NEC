#ifndef __CRC_LIB_H
#define __CRC_LIB_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
  
  uint8_t crc8_fast(const uint8_t *pcBlock, uint8_t len);  
  uint8_t canpro_crc8_fast(const uint8_t *pcBlock, uint8_t len);
  
  uint8_t tacho_crc(const uint8_t* data, uint16_t len);
  
  uint16_t crc16_fast(const uint8_t *pcBlock, uint16_t len);
  uint16_t crc16_slow(const uint8_t *pcBlock, uint16_t len);//legacy
  uint16_t crc16_modbus(const uint8_t *p, uint16_t len);
  
  void crc16_slow_init(uint16_t* crc);
  void crc16_slow_calc(uint16_t* crc, const uint8_t* buff, uint16_t len);
  void crc16_slow_finish(uint16_t* crc);
  uint16_t crc16_slow_full(const uint8_t* buff, uint16_t len);

  uint32_t crc32_fast_full(const uint8_t* buff, uint32_t len);
  void crc32_fast_init(uint32_t* crc);
  void crc32_fast_update(uint32_t* crc, const uint8_t* buff, uint32_t len);
  void crc32_fast_finish(uint32_t* crc);
  
  uint32_t crc32_slow_full(const uint8_t* buff, uint32_t len);
  void crc32_slow_init(uint32_t* crc);
  void crc32_slow_update(uint32_t* crc, const uint8_t* buff, uint32_t len);
  void crc32_slow_finish(uint32_t* crc);
  
#ifdef __cplusplus
}
#endif

#endif //__CRC_LIB_H

