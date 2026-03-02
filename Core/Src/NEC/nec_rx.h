#ifndef NEC_RX_H
#define NEC_RX_H
#include "main.h"

#define MARK_FRAME 9000.0
#define MARK_SPACE 4500.0
#define MARK_BIT   562.5
#define BIT_0      562.5
#define BIT_1      (3 * BIT_0)
#define REPEAT_SPACE (MARK_SPACE/2)

typedef enum
{
  NEC_IDLE,
  NEC_MARK_FRAME,
  NEC_SPACE,
  NEC_MARK_BIT,
  NEC_BIT,
  NEC_ADDRESS,
  NEC_COMMAND,
  NEC_ERROR,
  NEC_END
}nec_fsm_state_e;

typedef struct {
 nec_fsm_state_e state, 
 uint16_t mark_time;
 uint16_t space_time;
 uint8_t  address;  
 uint8_t  n_address; 
 uint8_t  command;  
 uint8_t  n_command; 
}nec_t;



#endif //NEC_RX_H