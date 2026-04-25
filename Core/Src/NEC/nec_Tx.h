#ifndef NEC_TX_H
#define NEC_TX_H
#include "main.h"
#include <stdbool.h>
#include "nec_Rx.h"


typedef enum
{
  NEC_TX_IDLE,
  NEC_TX_START,
  NEC_TX_SPACE,
  NEC_TX_MARK_BIT,
  NEC_TX_BIT,
  NEC_TX_END
}nec_TX_state_e;

typedef struct {
 nec_TX_state_e        state; 

 struct{
    uint8_t                address;  
    uint8_t                command; 
    bool                   repeat;
 }current;
 
  struct{
    uint8_t                address;  
    uint8_t                command;  
    bool                   repeat;
 }pending;
 uint8_t                bit_cnt;
 bool                   launch;
 bool                   active;
 bool                   timer_done;
// bool                   complete;

// bool                   fast_next_state;
}nec_Tx_t;

void NEC_TX_OutInit(void);
void NEC_TX_TimerInit(void);
void NEC_TX_Proced(void);
void NEC_TX_SendCommand(uint8_t _addr, uint8_t _comm, bool _repeat);
void NEC_TX_Out_Test();
#endif