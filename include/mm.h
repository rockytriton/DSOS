#ifndef	_MM_H
#define	_MM_H

#include "peripherals/base.h"

#define PAGE_SHIFT	 	12
#define TABLE_SHIFT 		9
#define SECTION_SHIFT		(PAGE_SHIFT + TABLE_SHIFT)

#define PAGE_SIZE   		(1 << PAGE_SHIFT)	
#define SECTION_SIZE		(1 << SECTION_SHIFT)	

#define LOW_MEMORY              (2 * SECTION_SIZE)
#define HIGH_MEMORY             PBASE

#define PAGING_MEMORY           (HIGH_MEMORY - LOW_MEMORY)
#define PAGING_PAGES            (PAGING_MEMORY/PAGE_SIZE) / 2

#ifndef __ASSEMBLER__

#ifdef __cplusplus 
extern "C" {
#endif

void memzero(unsigned long src, unsigned long n);
unsigned long allocPage();
void freePage(unsigned long);

extern int numPages;

void memcpy(unsigned char *dest, unsigned char *src, int size);

void writeMmio(uint32_t reg, uint32_t data);
uint32_t readMmio(uint32_t reg);

#ifdef __cplusplus 
}
#endif

#endif

#endif  /*_MM_H */
