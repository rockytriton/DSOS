#include <sys/irq.h>
#include <peripherals/irq.h>

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

typedef struct {
	reg32 basicPending;
	reg32 pending[2];
	reg32 fiqControl;
	reg32 enable[2];
	reg32 enableBasic;
	reg32 disable[2];
	reg32 disableBasic;
} IRQRegisters;

static IRQ_HANDLER handlers[32];
static void *userData[32];

void enable_interrupt_controller()
{
	//put32(ENABLE_IRQS_1, SYSTEM_TIMER_IRQ_1 | SYSTEM_TIMER_IRQ_3 | AUX_IRQ | (1 << 9));
}

void show_invalid_entry_message(int type, unsigned long esr, unsigned long address)
{
	log_println("INVALID ENTRY ERROR");
	log_print("%s, ESR: %x, address: %x\r\n", entry_error_messages[type], esr, address);
}

void irq_set_handler(SysIrq irq, IRQ_HANDLER handler, void *ud) {
    IRQRegisters *IRQ = (IRQRegisters *)(PBASE+0x0000B200);

    handlers[irq] = handler;
    userData[irq] = ud;
    IRQ->enable[0] |= (1 << irq);
}

void handle_irq() {
    IRQRegisters *IRQ = (IRQRegisters *)(PBASE+0x0000B200);
    dword irq = IRQ->pending[0];
    dword orig = irq;

    for (int i=0; i<32; i++) {
        bool hit = ((1 << i) & irq) != 0;
        IRQ_HANDLER handler = handlers[i];

        if (hit && !handler) {
            log_println("No handler for IRQ: %d", (dword)i);
            irq &= ~(1 << i);
        } else if (hit) {
            //log_println("HANDLING IRQ: %d", (dword)i);
            handler((SysIrq)i, userData[i]);
            irq &= ~(1 << i);
        }
    }
}

void handle_uart_irq();

void handle_irq2(void)
{
	unsigned int irq = get32(IRQ_PENDING_1);
	int orig = irq;

	while(irq) {
		if (irq & SYSTEM_TIMER_IRQ_1) {
			handle_timer_irq();
			irq &= ~SYSTEM_TIMER_IRQ_1;
		} else if (irq & SYSTEM_TIMER_IRQ_3) {
			handle_timer_irq2();
			irq &= ~SYSTEM_TIMER_IRQ_3;
		} else if (irq & AUX_IRQ) {
			//log_println("AUX IRQ");
			handle_uart_irq();
			irq &= ~AUX_IRQ;
		} else if (irq) {
			log_print("Unknown pending irq: %x - %x\r\n", irq, orig);
			break;
		} else {
			log_print("Zero pending irq: %x - %x\r\n", irq, orig);
			break;
		}
	}
	
}


