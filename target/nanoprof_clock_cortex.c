


//----- separate file  nanoprof_clock_cortex.c

// could use dummy clock with just increase index


uint32_t nanoprof_clock_get_timestamp(void)
{
  return SysTick->VAL;
}


void nanoprof_clock_init(void)
{
  const uint32_t ticks = (CPU_CLOCK_HZ/8) / CONFIG_PREEMPT_TIMER_FREQ;
  
  SysTick->LOAD  = (ticks & SysTick_LOAD_RELOAD_Msk) - 1; // set reload reg
  SysTick->VAL   = 0;
  SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk;
  SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
  
  SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}
