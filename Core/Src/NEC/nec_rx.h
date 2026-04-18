#ifndef NEC_RX_H
#define NEC_RX_H
#include "main.h"
#include <stdbool.h>

#define MARK_FRAME 9000.0
#define MARK_SPACE 4500.0
#define MARK_BIT   562.5
#define BIT_0      562.5
#define BIT_1      (3 * BIT_0)
#define REPEAT_SPACE (MARK_SPACE/2)

typedef enum
{
  NEC_IDLE,
  NEC_START,
  NEC_SPACE,
  NEC_MARK_BIT,
  NEC_BIT,
//  NEC_ADDRESS,
//  NEC_COMMAND,
//  NEC_ERROR,
  NEC_END
}nec_fsm_state_e;

typedef struct {
 nec_fsm_state_e        state; 
 uint16_t               start_time;
 uint16_t               space_time;
 uint8_t                address;  
 uint8_t                n_address; 
 uint8_t                command;  
 uint8_t                n_command; 
 uint16_t               tim_cnt;       
 uint8_t                bit_cnt; 
 bool                   done;
}nec_t;

void NEC_RX_Init(void);
void NEC_RX_Poll(void);
bool NEC_RX_IsDone(void);
void NEC_RX_SetTick(void);

#endif //NEC_RX_H