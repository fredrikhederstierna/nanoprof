
#include <cortex.h>

#include <nanoprof_clock.h>

#define CPU_CLOCK_HZ  (180*1000*1000)

// systick counter is max 24 bits long
#define SYSTICK_COUNT_MAX (0x01000000U-1U)

static uint32_t systick_wraps = 0;
// ISR
void SysTick_Handler(void)
{
  /* ISR handler. */
  systick_wraps++;
}

void nanoprof_clock_start(void)
{
  SysTick->CTRL = 0;
  // load with max counter value, count down to zero
  SysTick->VAL  = SYSTICK_COUNT_MAX;
  // use max speed HCLK for systick (core speed), also enable IRQ
  SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

void nanoprof_clock_stop(void)
{
  SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

uint64_t nanoprof_clock_get_timestamp(void)
{
  // how many ticks have passed from start value
  volatile uint32_t systick = SysTick->VAL;
  systick = (SYSTICK_COUNT_MAX - systick);
  // convert from HCLK to 1us ticks
  return (uint64_t)((1000ULL*1000ULL * (uint64_t)systick) / (CPU_CLOCK_HZ));
}

void nanoprof_clock_init(void)
{
  SysTick->LOAD = SYSTICK_COUNT_MAX;
  SysTick->VAL  = 0;
  // use max speed HCLK for systick (core speed)
  SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk;
}
