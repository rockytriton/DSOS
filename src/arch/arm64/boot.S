#include "arm/sysregs.h"
#include "mm.h"

#define OSC_FREQ		19200000

.section ".text.boot"

.globl _start
_start:
	mrs	x0, mpidr_el1
	// Check processor id	
	and	x0, x0, #0xFF 
	// Currently only uses single core CPU, hangs all other cores.
	cbz	x0, master		    
	b	proc_hang

proc_hang: 
	mov x3, x0
	mov x6, #1024
	mul x4, x3, x6
	add x5, x4, #LOW_MEMORY
	mov sp, x5

	bl proc_init
	mrs	x0, mpidr_el1
	and	x0, x0, #0xFF
	
	// Ok, just say hi to the extra cores.
	str x0, [sp, #-8]
	bl  proc_hi

	b	proc_hang_loop
proc_hang_loop:
	b 	proc_hang_loop

master:
	bl setupPeripheralBase

	ldr x0, =OSC_FREQ
	msr CNTFRQ_EL0, x0
	
	ldr x0, =SCTLR_VALUE_MMU_DISABLED
	msr sctlr_el1, x0

	ldr x0, =HCR_VALUE
	msr hcr_el2, x0

	ldr x0, =SCR_VALUE
	msr scr_el3, x0

	ldr x0, =SPSR_VALUE
	msr spsr_el3, x0

	adr x0, el1_entry
	msr elr_el3, x0

	eret

el1_entry:
	adr	x0, bss_begin
	adr	x1, bss_end
	sub	x1, x1, x0
	bl 	memzero

	mov	sp, #LOW_MEMORY 
	bl	kernel_main
	b 	proc_hang		// should never come here

.globl setDone
setDone:
	mov x2, #1
	bl okReset

.globl getEL
getEL:
	mrs x0, CurrentEL
	lsr x0, x0, #2
	ret

.globl getPN
getPN:
	mrs	x0, midr_el1
	//read primary part number
	lsr x0, x0, #4
	and x0, x0, #0x00000FFF
	ret

.global setupPeripheralBase
setupPeripheralBase:
	//default with PIB3 base
	ldr x0, =0x3F000000
	ldr x1, addr_PBASE
	str x0, [x1]

	//read cpu info..
	mrs	x0, midr_el1
	//read primary part number
	lsr x0, x0, #4
	and x0, x0, #0x00000FFF

	//0xD03 is PI3B
	//ldr x2, addr_CPUVER
	//str x0, [x2]

	cmp x0, #0xD08
	b.eq PI3B

	//otherwise assume PI4
PI4:
	ldr x0, =0xFE000000
	ldr x1, addr_PBASE
	str x0, [x1]
	
PI3B:	
	ret

.globl PBASE
PBASE: .dword

addr_PBASE : .dword PBASE
