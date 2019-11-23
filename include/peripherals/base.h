#ifndef	_P_BASE_H
#define	_P_BASE_H

#ifndef __ASSEMBLER__
#include <stdint.h>
extern unsigned int PBASE;
#endif


#define PERIPHERALS_BASE 0x20000000
//#define DWC_REGS_BASE          (PERIPHERALS_BASE + 0x980000)
#define DWC_REGS_BASE          (PBASE + 0x980000)

#endif  /*_P_BASE_H */
