#ifndef	_P_TIMER_H
#define	_P_TIMER_H


#include "peripherals/base.h"

#define CLOCKHZ	1000000

#define TIMER_CS        (PBASE+0x00003000)
#define TIMER_CLO       (PBASE+0x00003004)
#define TIMER_CHI       (PBASE+0x00003008)
#define TIMER_C0        (PBASE+0x0000300C)
#define TIMER_C1        (PBASE+0x00003010)
#define TIMER_C2        (PBASE+0x00003014)
#define TIMER_C3        (PBASE+0x00003018)
#define IRQ_USB         (PBASE+0x00003030)
//4 = 1C
//5 = 20
//6 = 24
//7 = 28
//8 = 2C
//9 = 30

#define TIMER_CS_M0	(1 << 0)
#define TIMER_CS_M1	(1 << 1)
#define TIMER_CS_M2	(1 << 2)
#define TIMER_CS_M3	(1 << 3)

#endif  /*_P_TIMER_H */
