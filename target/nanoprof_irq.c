
#include <nanoprof_irq.h>

#define IRQ_MASK_NORMAL  (1 << 0)

static volatile uint32_t interrupt_mask = 0;

//------------------------------------------
uint32_t nanoprof_irq_off(void)
{
  __disable_irq(); // inhibit any interrupt while setting up
  uint32_t old_mask = interrupt_mask;
  if ((old_mask & IRQ_MASK_NORMAL) == 0) {
    interrupt_mask |= IRQ_MASK_NORMAL;
    // disable all interrupts with preemption priority 1 and below [irq]
    __set_BASEPRI(0x80);
  }
  __enable_irq();
  return old_mask;
}

//------------------------------------------
void nanoprof_irq_restore(uint32_t new_mask)
{
  __disable_irq(); // inhibit any interrupt while setting up
  if ((interrupt_mask & IRQ_MASK_NORMAL) &&
      (new_mask & IRQ_MASK_NORMAL) == 0) {
    // enable all interrupts with preemption priority 1 and below [irq]
    __set_BASEPRI(0x00);
  }
  interrupt_mask = new_mask;
  __enable_irq();
}

bool nanoprof_irq_is_enabled(void)
{
  return false;
}

bool nanoprof_irq_context(void)
{
  return false;
}
