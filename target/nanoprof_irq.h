#ifndef __NANOPROF_IRQ_H__
#define __NANOPROF_IRQ_H__

#include <stdint.h>
#include <stdbool.h>

#define NANOPROF_IRQ_OFF            nanoprof_irq_off
#define NANOPROF_IRQ_RESTORE(flags) nanoprof_irq_restore(flags)
#define NANOPROF_IRQ_IS_ENABLED     nanoprof_irq_is_enabled
#define NANOPROF_IRQ_CONTEXT        nanoprof_irq_context

uint32_t nanoprof_irq_off(void);
void     nanoprof_irq_restore(uint32_t flags);
bool     nanoprof_irq_is_enabled(void);
bool     nanoprof_irq_context(void);

#endif
