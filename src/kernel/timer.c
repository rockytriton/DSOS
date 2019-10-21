#include "utils.h"
#include "printf.h"
#include "peripherals/timer.h"
#include "mm.h"
#include "scheduler.h"

const unsigned int interval = 200000;
const unsigned int interval2 = 200000 * 50;
unsigned int curVal = 0;
unsigned int curVal2 = 0;

int timersReceived = 0;
int timers2Received = 0;

void schedulerTick();

struct dwc_regs {

    /* 0x000 : OTG Control */
    unsigned int otg_control;

    /* 0x004 : OTG Interrupt */
    unsigned int otg_interrupt;

    /**
     * 0x008 : AHB Configuration Register.
     *
     * This register configures some of the interactions the DWC has with the
     * rest of the system.  */
    unsigned int ahb_configuration;

    /* 0x01c : Core USB configuration */
    unsigned int core_usb_configuration;

    /**
     * 0x010 : Core Reset Register.
     *
     * Software can use this register to cause the DWC to reset itself.
     */
    unsigned int core_reset;
};

void timer_init ( void )
{
	curVal = get32(TIMER_CLO);
	curVal2 = curVal + (interval2);
	curVal += interval;

	put32(TIMER_C1, curVal);
	put32(TIMER_C3, curVal2);
}

int countProcesses() {
	int c = 0;
	for (struct Task *p = first; p != 0; p = p->next) {
		c++;
	}
	return c;
}

void handle_timer_irq( void ) 
{
	curVal += (interval);
	put32(TIMER_C1, curVal);
	put32(TIMER_CS, get32(TIMER_CS) & TIMER_CS_M1);
	timersReceived++;

	if (timersReceived % 15 == 0) {
		
	}

	schedulerTick();
}

int getTemp();

void handle_timer_irq2( void ) 
{
	timers2Received++;
	curVal2 += (interval2);
	put32(TIMER_C3, curVal2);
	put32(TIMER_CS, get32(TIMER_CS) & TIMER_CS_M3);

	printf("Timer interrupts received %d\r\n", timersReceived);
	printf("Timer2 interrupts received %d\r\n", timers2Received);
	printf("Memory Usage:\r\n");
	printf("\tMax Memory : %d mb\r\n", PAGING_PAGES * PAGE_SIZE / 1024 / 1024);
	printf("\tUsed Memory: %d mb (%d kb)\r\n", numPages * PAGE_SIZE / 1024 / 1024, numPages * PAGE_SIZE / 1024);
	printf("\tFree Memory: %d mb\r\n", ((PAGING_PAGES * PAGE_SIZE) - (numPages * PAGE_SIZE)) / 1024 / 1024);
	printf("Processor Info:\r\n");
	printf("\tCurrent Process ID : %d\r\n", current->pId);
	printf("\tCurrent Process Adr: %X\r\n", current);
	printf("\tNumber of Processes: %d\r\n", countProcesses());
	printf ("\t TEMP: %d\r\n", getTemp());
	printf("\r\n");

}
/*
void _Unwind_Resume(void *p) {

}
void __gxx_personality_v0(void *p) {

}
*/