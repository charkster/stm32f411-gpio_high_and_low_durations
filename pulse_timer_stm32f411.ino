#define SerialUSB         Serial // this is an alias for the usb serial port
#define CLK_FREQ          96.005 // update this to the core clock frequency, in MHz
#define SINGLE_CLK_PERIOD 10.42  // updates this to the core clock period, in ns
#define NUM_CLK_PERIODS    2     // this is a fixed offset adjustment factor
#define ARRAY_LENGTH     100     // this is the number of consecutive high and low durations to be saved

volatile uint32_t cc1_array [ARRAY_LENGTH];
volatile uint32_t cc2_array [ARRAY_LENGTH];
volatile uint32_t cc1_count;
volatile uint32_t cc2_count;
volatile uint32_t cc1_count_orig;
volatile uint32_t cc2_count_orig;
volatile uint32_t a;
volatile uint32_t b;
volatile uint32_t high_duration;
volatile uint32_t low_duration;
volatile uint32_t array_length;
volatile char     serial_command;

void setup() {
  SerialUSB.begin(115200);                             // Send data back on the native port
  while(!SerialUSB);                                   // Wait for the SerialUSB port to be ready

  GPIOA->PUPDR  |= GPIO_PUPDR_PUPD5_1;                 // PA5 pull-down
  GPIOA->MODER  |= GPIO_MODER_MODE5_1;                 // PA5 in alternate function mode
  GPIOA->AFR[0] |= GPIO_AFRL_AFSEL5_0;                 // PA5 in alternate function mode AF1 (T2 CH1)

  RCC->APB1ENR  |= RCC_APB1ENR_TIM2EN;                  // enable TM2 timer
  TIM2->SMCR    |= TIM_SMCR_SMS_2 |                     // Counter will reset
                   TIM_SMCR_TS_2;                       // TRC will be TIM2_CH1 edge detect
  TIM2->EGR     |= TIM_EGR_CC1G     | TIM_EGR_CC2G;     // events are generated for capture counters 1 and 2
  TIM2->CCMR1   |= TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_1; // capture counter 1 is TI1, compare counter 2 is TI1
  TIM2->CCER    |= TIM_CCER_CC1E    | TIM_CCER_CC2E |   // enable counters 1 and 2
                   TIM_CCER_CC1P;                       // invert CC1 polarity
  TIM2->DIER    |= TIM_DIER_CC1IE | TIM_DIER_CC2IE;     // enable interrupts for CC1 and CC2 updates
  TIM2->CR1     |= TIM_CR1_CEN;                         // enable timer
  
  NVIC_SetPriority (TIM2_IRQn, 0);
  NVIC->ISER[0] |= (1 << TIM2_IRQn);                    // enable TIM2 int in NVIC
  
}

void loop() 
{
  if (SerialUSB.available())
  {
//    noInterrupts();
    serial_command = SerialUSB.read();
    if (serial_command == 'c')
    {
      cc1_count = 0;
      cc2_count = 0;
    }
    else
    { 
      cc1_count_orig = cc1_count;
      cc2_count_orig = cc2_count;
      if (cc1_count > 0)
      {
        array_length = cc1_count - 1;
        for ( a=0; a < array_length; a = a + 1 )
        {
          high_duration = (cc1_array[a] * (1000.0 / CLK_FREQ) + NUM_CLK_PERIODS*SINGLE_CLK_PERIOD);
          if (high_duration > 3900000000)
            {
              cc1_count--;
              for ( b=a; b < array_length; b = b + 1 )
              {
                cc1_array[b] = cc1_array[b+1];
              }
            }
        }
      }
      if (cc2_count > 0)
      {
        array_length = cc2_count - 1;
        for ( a=0; a < array_length; a = a + 1 )
        {
          low_duration = cc2_array[a] * (1000.0 / CLK_FREQ) + NUM_CLK_PERIODS*SINGLE_CLK_PERIOD;
          if (low_duration > 3900000000)
          {
            cc2_count--;
            for ( b=a; b < array_length; b = b + 1 )
            {
              cc2_array[b] = cc2_array[b+1];
            }
          }
        }
      }
      SerialUSB.print("pos_pulses_count=");
      SerialUSB.println(cc1_count_orig);
      SerialUSB.print("neg_pulses_count=");
      SerialUSB.println(cc2_count_orig);

      for ( a=0; a < cc1_count; a = a + 1 )
      {
        high_duration = cc1_array[a] * (1000.0 / CLK_FREQ) + NUM_CLK_PERIODS*SINGLE_CLK_PERIOD;
        SerialUSB.print("high_duration=");
        SerialUSB.println(high_duration);
        if (a < cc2_count)
        {
          low_duration = cc2_array[a] * (1000.0 / CLK_FREQ) + NUM_CLK_PERIODS*SINGLE_CLK_PERIOD;
          SerialUSB.print("low_duration=");
          SerialUSB.println(low_duration);
        }
      }
      SerialUSB.print("--> input c to clear counts <--\n");
    }
  }
}

extern "C" void TIM2_IRQHandler(void)  // Interrupt Service Routine (ISR) for timer TIM2
{
  // Check for match counter 1 interrupt
  if (TIM2->SR & TIM_SR_CC1IF)
  {
     cc1_array[cc1_count++] = TIM2->CCR1; // status bit self clears on read
  }
  // Check for match counter 2 interrupt
  if (TIM2->SR & TIM_SR_CC2IF)
  {
     cc2_array[cc2_count++] = TIM2->CCR2; // status bit self clears on read
  }
}

// call this function if you want to see the system clock div5 on PA8
void en_pll_div5_on_pa8(void)
{
  GPIOA->MODER |= GPIO_MODER_MODE8_1;                       // alternate function on PA8, by default it is MCO1
  RCC->CFGR    |= RCC_CFGR_MCO1_0    | RCC_CFGR_MCO1_1    | // select PLL
                  RCC_CFGR_MCO1PRE_2 | RCC_CFGR_MCO1PRE_1 | // select divide by 5
                  RCC_CFGR_MCO1PRE_0;
}
