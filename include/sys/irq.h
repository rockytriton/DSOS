#pragma once

#include <common.h>


#define S_FRAME_SIZE			256 

#define SYNC_INVALID_EL1t		0 
#define IRQ_INVALID_EL1t		1 
#define FIQ_INVALID_EL1t		2 
#define ERROR_INVALID_EL1t		3 

#define SYNC_INVALID_EL1h		4 
#define IRQ_INVALID_EL1h		5 
#define FIQ_INVALID_EL1h		6 
#define ERROR_INVALID_EL1h		7 

#define SYNC_INVALID_EL0_64	    	8 
#define IRQ_INVALID_EL0_64	    	9 
#define FIQ_INVALID_EL0_64		10 
#define ERROR_INVALID_EL0_64		11 

#define SYNC_INVALID_EL0_32		12 
#define IRQ_INVALID_EL0_32		13 
#define FIQ_INVALID_EL0_32		14 
#define ERROR_INVALID_EL0_32		15 

#define ARM_IRQS_PER_REG	32
#define ARM_IRQS_BASIC_REG	8

#define ARM_IRQ1_BASE		0
#define ARM_IRQ2_BASE		(ARM_IRQ1_BASE + ARM_IRQS_PER_REG)
#define ARM_IRQBASIC_BASE	(ARM_IRQ2_BASE + ARM_IRQS_PER_REG)
#define ARM_IRQLOCAL_BASE	(ARM_IRQBASIC_BASE + ARM_IRQS_BASIC_REG)

#define ARM_IRQ_TIMER0		(ARM_IRQ1_BASE + 0)
#define ARM_IRQ_TIMER1		(ARM_IRQ1_BASE + 1)
#define ARM_IRQ_TIMER2		(ARM_IRQ1_BASE + 2)
#define ARM_IRQ_TIMER3		(ARM_IRQ1_BASE + 3)
#define ARM_IRQ_CODEC0		(ARM_IRQ1_BASE + 4)
#define ARM_IRQ_CODEC1		(ARM_IRQ1_BASE + 5)
#define ARM_IRQ_CODEC2		(ARM_IRQ1_BASE + 6)
#define ARM_IRQ_JPEG		(ARM_IRQ1_BASE + 7)
#define ARM_IRQ_ISP		(ARM_IRQ1_BASE + 8)
#define ARM_IRQ_USB		(ARM_IRQ1_BASE + 9)
#define ARM_IRQ_3D		(ARM_IRQ1_BASE + 10)
#define ARM_IRQ_TRANSPOSER	(ARM_IRQ1_BASE + 11)
#define ARM_IRQ_MULTICORESYNC0	(ARM_IRQ1_BASE + 12)
#define ARM_IRQ_MULTICORESYNC1	(ARM_IRQ1_BASE + 13)
#define ARM_IRQ_MULTICORESYNC2	(ARM_IRQ1_BASE + 14)
#define ARM_IRQ_MULTICORESYNC3	(ARM_IRQ1_BASE + 15)

#define ARM_IRQLOCAL0_CNTPS	(ARM_IRQLOCAL_BASE + 0)
#define ARM_IRQLOCAL0_CNTPNS	(ARM_IRQLOCAL_BASE + 1)
#define ARM_IRQLOCAL0_CNTHP	(ARM_IRQLOCAL_BASE + 2)
#define ARM_IRQLOCAL0_CNTV	(ARM_IRQLOCAL_BASE + 3)

typedef enum {
    SITimer0 = 0,
    SITimer1 = 1,
    SITimer2 = 2,
    SITimer3 = 3,
    SIUsb = 9,
    SIAux = 29
} SysIrq;

typedef void (*IRQ_HANDLER)(SysIrq, void *);

void init();

void irq_set_handler(SysIrq irq, IRQ_HANDLER handler, void *userData);


void enable_interrupt_controller( void );

void irq_vector_init( void );
void enable_irq( void );
void disable_irq( void );

