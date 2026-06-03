#include "nec_Rx.h"
#include "tim.h"
#include "gpio.h"

nec_t NEC_RX = {  .state = NEC_IDLE, .fast_next_state = false};

typedef struct {
uint16_t min;
uint16_t max;
}range_t;

typedef struct {
  range_t start;
  range_t space;
  range_t repeat;
  range_t mark_bit;
  range_t bit_0;
  range_t bit_1;

}range_pulse_t;

range_pulse_t RangeNEC={
.start  = { 8200, 9500 },
.space  = { 4200, 4600 },
.repeat = { 2100, 2500},
.mark_bit={ 450, 600 },
.bit_0  = { 450, 600 },
.bit_1  = { 1200, 2000 }
};

uint16_t arr_tim_cnt[128];

void NEC_RX_SetState(nec_fsm_state_e _st)
{
  NEC_RX.state = _st;
  if( NEC_END == _st){
     NEC_RX.fast_next_state = true;
  }
}

  
void  NEC_RX_Set_Trigger(uint8_t Trigger)
{
  switch(Trigger)
  {
    case LL_EXTI_TRIGGER_RISING:
      LL_EXTI_DisableFallingTrig_0_31(LL_EXTI_LINE_5);
      LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_5);
      break;
    case LL_EXTI_TRIGGER_FALLING:
      LL_EXTI_DisableRisingTrig_0_31(LL_EXTI_LINE_5);
      LL_EXTI_EnableFallingTrig_0_31(LL_EXTI_LINE_5);
      break;
  }
}

  
void NEC_RX_Init(void)
{
    NEC_RX.state = NEC_IDLE;
    NEC_RX.bit_cnt = 0;
    NEC_RX.tim_cnt = 0;
    NEC_RX.indx = 0;
    NEC_RX.done = false;
    NEC_RX_Set_Trigger(LL_EXTI_TRIGGER_FALLING);
    NEC_RX_TimerStop();
}

bool NEC_RX_IsComplete(void)
{
  return NEC_RX.complete;
}
bool NEC_RX_IsRepeat(void)
{
  return NEC_RX.repeat ;
}

void NEC_RX_CompleteReset(void)
{
  NEC_RX.complete = false;
  NEC_RX.repeat   = false;      
}

void NEC_RX_SetTick(void)
{
  if(NEC_RX.state == NEC_IDLE)
    NEC_RX.tim_cnt = 1;
  else
    NEC_RX.tim_cnt = LL_TIM_GetCounter(TIM16);
}

void NEC_RX_Poll(void)
{
  NEC_RX_TimerStop();
  NEC_RX_SetTick();
    
  if(NEC_RX.tim_cnt)
  {

    do{
        NEC_RX.fast_next_state = false;
        switch(NEC_RX.state)
        {
          case NEC_IDLE:
            {
              NEC_RX.tim_cnt = 0;
              NEC_RX_Set_Trigger(LL_EXTI_TRIGGER_RISING);
              NEC_RX_SetState(NEC_START);
            }
            break;
            
          case NEC_START:
            {
              NEC_RX_Set_Trigger(LL_EXTI_TRIGGER_FALLING);
              arr_tim_cnt[NEC_RX.indx++] = NEC_RX.tim_cnt;
              if(NEC_RX.tim_cnt < RangeNEC.start.max && NEC_RX.tim_cnt >= RangeNEC.start.min  ){
                NEC_RX_SetState(NEC_SPACE);
              }
              else{
                NEC_RX_SetState(NEC_END);
              }
            }
            break;
            
          case NEC_SPACE:
            {
              NEC_RX_Set_Trigger(LL_EXTI_TRIGGER_RISING);
              arr_tim_cnt[NEC_RX.indx++] = NEC_RX.tim_cnt;
              if(NEC_RX.tim_cnt < RangeNEC.space.max && NEC_RX.tim_cnt >= RangeNEC.space.min  ){
                NEC_RX_SetState(NEC_MARK_BIT);
              }
              else if(NEC_RX.tim_cnt < RangeNEC.repeat.max && NEC_RX.tim_cnt >= RangeNEC.repeat.min){
                NEC_RX.repeat = true;
                NEC_RX_SetState(NEC_MARK_BIT);
              }
              else{
                NEC_RX_SetState(NEC_END);
              }
            }
            break;
            
          case NEC_MARK_BIT:
            {
              NEC_RX_Set_Trigger(LL_EXTI_TRIGGER_FALLING);
              arr_tim_cnt[NEC_RX.indx++] = NEC_RX.tim_cnt;
              if(NEC_RX.tim_cnt < RangeNEC.mark_bit.max && NEC_RX.tim_cnt >= RangeNEC.mark_bit.min  )
              {
                if(NEC_RX.bit_cnt <= 31 && !NEC_RX.repeat){
                  NEC_RX_SetState(NEC_BIT);
                }
                else{
                  NEC_RX.done = true;
                  NEC_RX_SetState(NEC_END);  
                }
              }
              else{
                NEC_RX_SetState(NEC_END);
              }
            }
              break;
              
          case NEC_BIT:
            {
              NEC_RX_Set_Trigger(LL_EXTI_TRIGGER_RISING);
              
              arr_tim_cnt[NEC_RX.indx++] = NEC_RX.tim_cnt;
              
              if(NEC_RX.tim_cnt < RangeNEC.bit_0.max && NEC_RX.tim_cnt >= RangeNEC.bit_0.min  ){
                NEC_RX_SetState(NEC_MARK_BIT);
                NEC_RX.bit_cnt++;
              }
              else if(NEC_RX.tim_cnt < RangeNEC.bit_1.max && NEC_RX.tim_cnt >= RangeNEC.bit_1.min )
              {
                NEC_RX_SetState(NEC_MARK_BIT);
                NEC_RX.bit_cnt++;
              } 
              else{
                NEC_RX_SetState(NEC_END);
              }
            }
            break;

          case NEC_END:
            {
              if(NEC_RX.done == true)
              {
                if(NEC_RX.repeat )
                {
                  NEC_RX.complete = true;
                }
                else{
                  for(uint8_t i=3, bit_cnt=0; bit_cnt<32; i+=2, bit_cnt++)
                  {
                     if(arr_tim_cnt[i] < RangeNEC.bit_0.max && arr_tim_cnt[i] >= RangeNEC.bit_0.min  )
                      {
                        CLEAR_BIT(NEC_RX.REG, (1<<bit_cnt)) ;
                      }
                      else if(arr_tim_cnt[i] < RangeNEC.bit_1.max && arr_tim_cnt[i] >= RangeNEC.bit_1.min )
                      {
                        SET_BIT(NEC_RX.REG, (1<<bit_cnt)) ;
                      } 
                  }
                  
                  volatile uint8_t com = NEC_RX.fild.command ;
                  volatile uint8_t n_com = ~NEC_RX.fild.n_command;
                  volatile uint8_t adr = NEC_RX.fild.address ;
                  volatile uint8_t n_adr = ~NEC_RX.fild.n_address;
                  
                  if((com == n_com ) && (adr == n_adr)){
                    NEC_RX.complete = true;
                  }
                }
              }
             NEC_RX_Init();
            }
            break;
        }
    }while(NEC_RX.fast_next_state);
  }
  NEC_RX_TimerReset();
  NEC_RX_TimerStart();
}

uint8_t NEC_RX_Get_Command(void)
{
  return NEC_RX.fild.command;
}

uint8_t NEC_RX_Get_Address(void)
{
  return NEC_RX.fild.address;
}


void NEC_RX_TimerInit(void)
{
  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_TIM16);

  NVIC_SetPriority(TIM16_IRQn, 0);
  NVIC_EnableIRQ(TIM16_IRQn);

  TIM_InitStruct.Prescaler = 48;// 1 тик = 1 мкс
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 30000;// 30мс
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  TIM_InitStruct.RepetitionCounter = 0;
  LL_TIM_Init(TIM16, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM16);
}

void NEC_RX_Input_Init(void)
{
  LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE5);

  LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_5, LL_GPIO_PULL_NO);
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_5, LL_GPIO_MODE_INPUT);
  /**/
  EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_5;
  EXTI_InitStruct.LineCommand = ENABLE;
  EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
  EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;//LL_EXTI_TRIGGER_RISING_FALLING;
  LL_EXTI_Init(&EXTI_InitStruct);

  /* EXTI interrupt init*/
  NVIC_SetPriority(EXTI4_15_IRQn, 0);
  NVIC_EnableIRQ(EXTI4_15_IRQn);
}


void NEC_RX_TimerStart(void)
{
   LL_TIM_ClearFlag_UPDATE(TIM16);
   LL_TIM_EnableIT_UPDATE(TIM16);
   LL_TIM_EnableCounter(TIM16);
}

 void NEC_RX_TimerStop(void)
 {
   LL_TIM_DisableCounter(TIM16);
   LL_TIM_DisableIT_UPDATE(TIM16);
 }



void NEC_RX_TimerReset(void)
{
  LL_TIM_SetCounter(TIM16, 0);
}
