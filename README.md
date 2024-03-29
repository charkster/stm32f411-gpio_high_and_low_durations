# stm32f411-gpio_high_and_low_durations
![picture](https://cdn2.botland.store/97774-pdt_540/stm32f411ceu6-blackpill-v31-development-board-with-stm32f411ceu6-microcontroller-weact-studio.jpg)

This arduino code for STM32F411 will record a gpio's high and low duration for multiple pulses. With a 100MHz system clock this can reliably capture 400ns high and low pulses (back-to-back).

The build_opt.h is needed to disable HAL timer irq handlers (I use a custom irq handler).
There are only a couple of gpio pins that connect to the TIM2 block as CH1 input (A5 and A0), I chose A5 and it is part of the code.

There are 2 parameters to trim to the system clock, CLK_FREQ and NUM_CLK_PERIODS. For greater accuracy, select appropriate values.
First calibrate with slow duration pulses, the CLK_FREQ will be adjusted. Next very fast durations (300ns and less) can be used to select NUM_CLK_PERIODS.

Added "en_pll_div5_on_pa8" function to output the PLL (system) clock div5 on PA8. This can be used to measure the exact CLK_FREQ.

Added uf2 file for easy programming. Tinyuf2 for stm32f411ce "Blackpill" needs to be programmed as the bootloader in order to use the uf2 file.


![picture](https://github.com/charkster/stm32f411-gpio_high_and_low_durations/blob/main/stm32f411_timer.png)
