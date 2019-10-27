#include "mini_uart.h"
#include "printf.h"
#include "utils.h"
#include "irq.h"
#include "scheduler.h"
#include "fork.h"
#include "mm.h"
#include "mailbox.h"

void putc ( void* p, char c );
void setDone();
int getEL();
void setEL1();
void doVideoCheck();
void _Z10cpp_kernelv();

//void timer_init ( void );
void handle_timer_irq ( void );
void handle_timer_irq2 ( void );

int kernel_loading = 1;

void kernel_main() {
	_Z10cpp_kernelv();
}

void kernel_main2() {
    //uart_init();
	init_printf(0, putc);
	printf("\r\n\r\nDreamSys OS 0.002 Loading...\r\n");
	irq_vector_init();
	printf("IRQ Vectors Initialized\r\n");
	//timer_init();
	printf("Timer Initialized\r\n");
	enable_interrupt_controller();
	printf("Interrupt Controller Enabled\r\n");
	enable_irq();
	printf("IRQ Enabled (%x, %x)\r\n", current->prev, current->next);
	
	printf("Kernel loaded, EL: %d\r\n", getEL());
    kernel_loading = 0;

    //doVideoCheck();

}


void proc_init() {
	while(kernel_loading) {
	
	}
}

void proc_hi(int p) 
{
	delay(100000 * p);
}

void okReset() {
	printf("DO RESET\n");
}

