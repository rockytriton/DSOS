#include "peripherals/timer.h"

.globl put32
put32:
	str w1,[x0]
	ret

.globl get32
get32:
	ldr w0,[x0]
	ret

.globl delay
delay:
	subs x0, x0, #1
	bne delay
	ret

.globl getId
getId:
	mrs	x0, midr_el1
	lsr x0, x0, #4
	ret

.globl clock_get_ticks
clock_get_ticks:
	ldr x4, =CLOCKHZ
	mrs x1, CNTPCT_EL0
	mrs x2, CNTFRQ_EL0
	mul x3, x1, x4
	sdiv x0, x3, x2
	ret

.globl clock_physical_ticks
clock_physical_ticks:
	ldr x4, =100
	mrs x0, CNTFRQ_EL0
	//sdiv x0, x1, x4
	ret

.globl clock_init
clock_init:
	ldr x4, =100
	mrs x1, CNTFRQ_EL0
	sdiv x2, x1, x4

	mrs x3, CNTPCT_EL0
	add x5, x2, x3
	msr CNTP_CVAL_EL0, x5
	ldr x6, =1
	msr CNTP_CTL_EL0, x6
	ret 
