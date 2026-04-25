#include "nec_Tx.h"
#include "tim.h"
#include "gpio.h"

#define NEC_TX_PIN              LL_GPIO_PIN_2


nec_Tx_t NEC_TX;

void NEC_TX_LOW()
{
  LL_GPIO_ResetOutputPin(GPIOA, NEC_TX_PIN);
}

void NEC_TX_HIGH()
{
  LL_GPIO_SetOutputPin(GPIOA, NEC_TX_PIN);
}

void NEC_TX_TimerInit(void)
{
  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM14);

  NVIC_SetPriority(TIM14_IRQn, 0);
  NVIC_EnableIRQ(TIM14_IRQn);

  TIM_InitStruct.Prescaler = 48;// 1 тик = 1 мкс
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 40000;// 40мс
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM14, &TIM_InitStruct);
//  LL_TIM_EnableARRPreload(TIM14);
  //___ PWM
//  LL_TIM_DisableARRPreload(TIM14);
//  LL_TIM_OC_EnablePreload(TIM14, LL_TIM_CHANNEL_CH1);
//  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
//  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
//  TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
//  TIM_OC_InitStruct.CompareValue = 0;
//  TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
//  LL_TIM_OC_Init(TIM14, LL_TIM_CHANNEL_CH1, &TIM_OC_InitStruct);
//  LL_TIM_OC_DisableFast(TIM14, LL_TIM_CHANNEL_CH1);
//  LL_TIM_SetOnePulseMode(TIM14, LL_TIM_ONEPULSEMODE_SINGLE);
  /* USER CODE BEGIN TIM14_Init 2 */

  /* USER CODE END TIM14_Init 2 */
//  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
//    /**TIM14 GPIO Configuration
//  PB1     ------> TIM14_CH1    */
//  GPIO_InitStruct.Pin = LL_GPIO_PIN_1;
//  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
//  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
//  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
//  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
//  GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
//  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void NEC_TX_OutInit(void)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

  GPIO_InitStruct.Pin = NEC_TX_PIN;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  NEC_TX_HIGH();
}


void NEC_TX_SendCommand(uint8_t _addr, uint8_t _comm, bool _repeat)
{
  NEC_TX.launch = true;
  NEC_TX.pending.command = _comm;
  NEC_TX.pending.address = _addr;
  NEC_TX.pending.repeat  = _repeat;
}


void NEC_TX_TimerStart_us(uint32_t _us)
{
  LL_TIM_DisableCounter(TIM14);
  LL_TIM_SetAutoReload(TIM14, _us);
  LL_TIM_SetCounter(TIM14, 0);
  LL_TIM_ClearFlag_UPDATE(TIM14);
  LL_TIM_EnableCounter(TIM14);
  LL_TIM_EnableIT_UPDATE(TIM14);

}

void NEC_TX_SetState(nec_TX_state_e _st)
{
  NEC_TX.state = _st;
}

void NEC_TX_Proced(void)
{
  if(NEC_TX.active == false)
  {
    if(NEC_TX.launch)
    {
      NEC_TX.active = true;
      NEC_TX.launch = false;
      NEC_TX.current.command = NEC_TX.pending.command;
      NEC_TX.current.address = NEC_TX.pending.address;
      NEC_TX.current.repeat  = NEC_TX.pending.repeat;
      NEC_TX.pending.command=0;
      NEC_TX.pending.address=0;
      NEC_TX.pending.repeat=0;
      NEC_TX.state = NEC_TX_START;
      NEC_TX.timer_done = true;
    }
  }
  
  if(NEC_TX.active && NEC_TX.timer_done)
  {

        NEC_TX.timer_done = false;
        switch(NEC_TX.state)
        {
          case NEC_TX_START:
             NEC_TX_LOW();
             NEC_TX_TimerStart_us(9000); 
             NEC_TX_SetState(NEC_TX_SPACE);
            break;
            
          case NEC_TX_SPACE:
           {
            NEC_TX_HIGH();
             uint32_t us = 4500;
             if(NEC_TX.current.repeat)
               us /= 2;
                         
             NEC_TX_TimerStart_us(us); 
             NEC_TX_SetState(NEC_TX_MARK_BIT);
        }
        break;
             
          case NEC_TX_MARK_BIT:
            
             NEC_TX_LOW();
             NEC_TX_TimerStart_us(560); 
             if(NEC_TX.bit_cnt >= 32 || NEC_TX.current.repeat){
               NEC_TX_SetState( NEC_TX_END );
              }
              else{
               
               NEC_TX_SetState(NEC_TX_BIT);  
              }
           break;
            
          case NEC_TX_BIT:
            {
             uint32_t us = 0;
             bool bit = false;
             NEC_TX_HIGH();
             if(NEC_TX.bit_cnt < 8){
              bit = 0x01&(NEC_TX.current.address >> NEC_TX.bit_cnt);
             }
             else if(NEC_TX.bit_cnt < 16){
              bit = 0x01&((~NEC_TX.current.address) >> (NEC_TX.bit_cnt-8));
             }
             else if(NEC_TX.bit_cnt < 24){
              bit = 0x01&(NEC_TX.current.command >> (NEC_TX.bit_cnt-16));
             }
             else {
              bit = 0x01&((~NEC_TX.current.command) >> (NEC_TX.bit_cnt-24));
             }
             NEC_TX.bit_cnt++;
             
             if(bit)
               us = 1680;
             else
               us = 560;
             
              NEC_TX_TimerStart_us(us); 
              NEC_TX_SetState(NEC_TX_MARK_BIT);  
            }
            break;
          case NEC_TX_END:
            {
              NEC_TX.current.command = 0;
              NEC_TX.current.address = 0;
              NEC_TX.bit_cnt = 0;
              NEC_TX.current.repeat = false;
              NEC_TX_HIGH();
              NEC_TX_TimerStart_us(20000); 
              NEC_TX_SetState(NEC_TX_IDLE);  
            }
            break;
            
          case NEC_TX_IDLE:
            NEC_TX.active = false;
            NEC_TX_HIGH();
            LL_TIM_DisableIT_UPDATE(TIM14);
          break;
           default: break;
        }

    }

}

void TIM14_IRQHandler(void)
{
  if(LL_TIM_IsActiveFlag_UPDATE(TIM14) )
  {
    LL_TIM_ClearFlag_UPDATE(TIM14);
    LL_TIM_DisableCounter(TIM14);
    NEC_TX.timer_done = true;
  }
}

void NEC_TX_Out_Test()
{
  for(uint8_t i=0; i<10; i++)
  {
    NEC_TX_HIGH();
    NEC_TX_LOW();
  }

}