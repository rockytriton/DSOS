#include "mm.h"
#include "log.h"

void dmb();

static unsigned short mem_map [ 0x3ec00 /*was PAGING_PAGES */ ] = {0,};

int numPages = 0;


unsigned long allocPage()
{
	for (int i = 0; i < PAGING_PAGES; i++){
		if (mem_map[i] == 0){
			mem_map[i] = 1;
            numPages++;
			log_println("CALLING allocPage: %X", LOW_MEMORY + i*PAGE_SIZE);
			return LOW_MEMORY + i*PAGE_SIZE;
		}
	}

	return 0;
}

void freePage(unsigned long p){
	mem_map[(p - LOW_MEMORY) / PAGE_SIZE] = 0;
    numPages--;
}

void memcpy(unsigned char *dest, unsigned char *src, int size) {
    for (int i=0; i<size; i++) {
        dest[i] = src[i];
    }
}

void writeMmio(uint32_t reg, uint32_t data) {
	dmb();
	*(volatile uint32_t *) (reg) = data;
	dmb();
}

uint32_t readMmio(uint32_t reg) {
	dmb();
	return *(volatile uint32_t *) (reg);
	dmb();
}
