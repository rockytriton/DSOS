#include "mini_uart.h"
#include "printf.h"
#include "utils.h"
#include "timer.h"
#include "irq.h"
#include "scheduler.h"
#include "fork.h"
#include "mm.h"
#include "mailbox.h"
#include <stdint.h>

#pragma pack(1)

#define PACKED
#define FF		__attribute__ ((packed))

void putc ( void* p, char c );
void setDone();
int getEL();
void setEL1();
void doVideoCheck();

int kernel_loading = 1;

typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;

struct CHSAddress
{
	byte head;
	byte sectorcylinderHigh;
	byte cylinderLow;
}PACKED;

struct PartitionEntry
{
	byte status;
	struct CHSAddress firstSector;
	byte type;
	struct CHSAddress lastSector;
	dword firstLBASector;
	dword numSectors;
} PACKED;

struct MasterBootRecord
{
	byte bootCode[0x1BE];
	struct PartitionEntry partitions[4];
	word bootSignature;
} PACKED;

int getId();

void kernel_main() {
    uart_init();
	init_printf(0, putc);
	printf("\r\n\r\nDreamSys OS 0.002 Loading...\r\n");
	printf("EL: %d\r\n", getEL());
	irq_vector_init();
	printf("IRQ Vectors Initialized\r\n");
	timer_init();
	printf("Timer Initialized\r\n");
	enable_interrupt_controller();
	printf("Interrupt Controller Enabled\r\n");
	enable_irq();
	printf("IRQ Enabled (%x, %x)\r\n", current->prev, current->next);
	
	printf("PI MODEL ID: %x - %d - %x\r\n", getId() & 0xFFF, PAGING_PAGES, PAGING_PAGES);

	struct MasterBootRecord mbr;

	mbr.partitions[0].firstLBASector = 5;

	printf("LBA: %d - %d\r\n", mbr.partitions[0].firstLBASector, sizeof(struct MasterBootRecord));

	printf("Kernel loaded, EL: %d\r\n", getEL());
    kernel_loading = 0;

    doVideoCheck();

    while(1) {

    }
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
	
}

