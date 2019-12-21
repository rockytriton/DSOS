
#include "utils.h"
#include "timer.h"
#include <sys/irq.h>
#include "scheduler.h"
#include "fork.h"
#include "mm.h"
#include <mailbox/mailbox.h>
#include "devices.h"
#include <sys/timer.h>

#include <peripherals/base.h>
#include <log.h>
#include <common.h>
#include <lib/dsstring.h>
#include "../drivers/video/video.h"

#include "../utils/rash/rash.h"

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
	irq_vector_init();
	timer_init();
	enable_interrupt_controller();
	enable_irq();

	dev_load_devices();

	log_print("\r\n\r\nDreamSys OS 0.002 Loading...\r\n");
	
	log_hex(0x2233, 6);
	log_println(" ");

	dword cr = mailbox_clock_rate(CTCore);

	log_println("Clock Rate: %d", cr);
	

	char a[32];
	str_copy(a, "A STRING");
	log_println("Copied String: %s", a);
	
	log_print("PROD ID: %x - %d - %x - %s\r\n", getId() & 0xFFF, PAGING_PAGES, PAGING_PAGES, a);

	struct MasterBootRecord mbr;

	mbr.partitions[0].firstLBASector = 5;

	log_print("LBA: %d - %d\r\n", mbr.partitions[0].firstLBASector, sizeof(struct MasterBootRecord));

	log_print("Kernel loaded, EL: %d\r\n", getEL());

    kernel_loading = 0;

	rash_main();

    while(1) {

    }

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
	
}

void assert(bool b, const char *msg) {
	if (!b) {
		log_println(" ");
		log_println("SYSTEM HALT");
		log_println(msg);

		while(true) {
			asm volatile ("wfi");
		}
	}

}
