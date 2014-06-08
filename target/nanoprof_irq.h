#ifndef __NANOPROF_IRQ_H__
#define __NANOPROF_IRQ_H__

#include <stdint.h>

#define NANOPROF_IRQ_OFF            __irq_off
#define NANOPROF_IRQ_RESTORE(flag)  __irq_restore(flag)
#define NANOPROF_IRQ_IS_ENABLED     __irq_is_enabled
#define NANOPROF_IRQ_CONTEXT        __irq_context

#endif
