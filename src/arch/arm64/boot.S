#include "arm/sysregs.h"

#include "mm.h"

.section ".text.boot"

.globl _start
_start:
	mrs	x0, mpidr_el1	
	and	x0, x0,#0xFF		// Check processor id
	cbz	x0, master		// Hang for all non-primary CPU
	b	proc_hang

proc_hang: 
	mov x3, x0
	mov x6, #1024
	mul x4, x3, x6
	add x5, x4, #LOW_MEMORY
	mov sp, x5

	bl proc_init
	mrs	x0, mpidr_el1
	and	x0, x0,#0xFF
	str x0, [sp, #-8]

	bl  proc_hi
	b	proc_hang_loop
proc_hang_loop:
	b 	proc_hang_loop

master:
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

