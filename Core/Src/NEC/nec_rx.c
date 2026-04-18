#include "nec_rx.h"
#include "tim.h"
#include "gpio.h"

nec_t NEC_RX = {  .state = NEC_IDLE};

typedef struct {
uint16_t min;
uint16_t max;
}range_t;

typedef struct {
//  range_t start         = { 850,  950};
//  range_t space         ={.min = 440, .max = 460};
//  range_t mark_bit      ={.min = 215, .max = 235};
//  range_t bit_0           ={.min = 102, .max = 122 };
//  range_t bit_1           ={.min = 215, .max = 235 };
  range_t start;
  range_t space;
  range_t mark_bit;
  range_t bit_0;
  range_t bit_1;

}range_pulse_t;

range_pulse_t RangeNEC={
.start  = { 850, 950 },
.space  = { 440, 460 },
.mark_bit={ 215, 235 },
.bit_0  = { 102, 122 },
.bit_1  = { 215, 235 }
};

uint16_t arr_tim_cnt[128];
uint8_t indx=0;

void NEC_RX_SetState(nec_fsm_state_e _st)
{
  NEC_RX.state = _st;
}

void NEC_RX_Init(void)
{
    NEC_RX.state = NEC_IDLE;
    NEC_RX.bit_cnt = 0;
    NEC_RX.tim_cnt = 0;
    NEC_RX.done = false;
}

bool NEC_RX_IsDone(void)
{
  return NEC_RX.done;
}

void NEC_RX_SetTick(void)
{
  if(NEC_RX.state == NEC_IDLE)
    NEC_RX.tim_cnt = 1;
  else
    NEC_RX.tim_cnt = NEC_RX_GetTimerTick();
}

void NEC_RX_Poll(void)
{
    
  if(NEC_RX.tim_cnt)
  {
    uint8_t pin_state = GetRxPinState();
    
    switch(NEC_RX.state)
    {
      case NEC_IDLE:
        if(pin_state == 0){
          NEC_RX.tim_cnt = 0;
          NEC_RX_TimerReset();
          NEC_RX_TimerStart();
          NEC_RX_SetState(NEC_START);
        }
        break;
      case NEC_START:
        if(pin_state == 1){
          arr_tim_cnt[indx++] = NEC_RX.tim_cnt;
//          NEC_RX_TimerReset();
//          NEC_RX_TimerStart();
          NEC_RX_SetState(NEC_SPACE);
        }
        break;
      case NEC_SPACE:
        if(pin_state == 0){
          arr_tim_cnt[indx++] = NEC_RX.tim_cnt;
//          NEC_RX_TimerReset();
//          NEC_RX_TimerStart();
          NEC_RX_SetState(NEC_MARK_BIT);
        }
        break;
      case NEC_MARK_BIT:
        if(pin_state == 1){
          arr_tim_cnt[indx++] = NEC_RX.tim_cnt;
//          NEC_RX_TimerReset();
//          NEC_RX_TimerStart();
          NEC_RX_SetState(NEC_BIT);
        }
        break;
      case NEC_BIT:
        if(pin_state == 0){
          arr_tim_cnt[indx++] = NEC_RX.tim_cnt;
          if(NEC_RX.bit_cnt++ <8*4){
            NEC_RX_SetState(NEC_MARK_BIT);
          }
          else{
           NEC_RX_SetState(NEC_END);
          }
//          NEC_RX_TimerReset();
//          NEC_RX_TimerStart();
 
        }
        break;
     
      case NEC_END:
         NEC_RX_Init();
//         for(uint8_t i=0; i<indx;i++){
//         
//         }
        break;
    }
    NEC_RX.tim_cnt = 0;
  }
}

