
#include <nanoprof_clock.h>

static volatile uint32_t __nanoprof_clock_index = 0;

uint32_t nanoprof_clock_get_timestamp(void)
{
  return __nanoprof_clock_index++;
}

void nanoprof_clock_init(void)
{
  __nanoprof_clock_index = 0;
}
