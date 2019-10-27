#include "utils.h"
#include "printf.h"
#include "entry.h"
#include "peripherals/irq.h"

const char *entry_error_messages[] = {
	"SYNC_INVALID_EL1t",
	"IRQ_INVALID_EL1t",		
	"FIQ_INVALID_EL1t",		
	"ERROR_INVALID_EL1T",		

	"SYNC_INVALID_EL1h",		
	"IRQ_INVALID_EL1h",		
	"FIQ_INVALID_EL1h",		
	"ERROR_INVALID_EL1h",		

	"SYNC_INVALID_EL0_64",		
	"IRQ_INVALID_EL0_64",		
	"FIQ_INVALID_EL0_64",		
	"ERROR_INVALID_EL0_64",	

	"SYNC_INVALID_EL0_32",		
	"IRQ_INVALID_EL0_32",		
	"FIQ_INVALID_EL0_32",		
	"ERROR_INVALID_EL0_32"	
};
void timer_init ( void );
void handle_timer_irq ( void );
void handle_timer_irq2 ( void );

void enable_interrupt_controller()
{
	put32(ENABLE_IRQS_1, SYSTEM_TIMER_IRQ_1 | SYSTEM_TIMER_IRQ_3 | AUX_IRQ | (1 << 9));
}

void show_invalid_entry_message(int type, unsigned long esr, unsigned long address)
{
	printf("%s, ESR: %x, address: %x\r\n", entry_error_messages[type], esr, address);
}

void handle_uart_irq();

void handle_irq(void)
{
	unsigned int irq = get32(IRQ_PENDING_1);
	int orig = irq;

	while(irq) {
		if (irq & SYSTEM_TIMER_IRQ_0) {
			handle_timer_irq();
			irq &= ~SYSTEM_TIMER_IRQ_0;
		} else if (irq & SYSTEM_TIMER_IRQ_1) {
			handle_timer_irq();
			irq &= ~SYSTEM_TIMER_IRQ_1;
		} else if (irq & SYSTEM_TIMER_IRQ_3) {
			handle_timer_irq();
			irq &= ~SYSTEM_TIMER_IRQ_3;
		} else if (irq & AUX_IRQ) {
			handle_uart_irq();
			irq &= ~AUX_IRQ;
		} else if (irq) {
			printf("Unknown pending irq: %x - %x\r\n", irq, orig);
			break;
		} else {
			printf("Zero pending irq: %x - %x\r\n", irq, orig);
			break;
		}
	}
	
}
