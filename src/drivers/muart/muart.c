#include "muart.h"
#include "peripherals/base.h"
#include "../gpio/gpio.h"

#define IER_REG_EN_REC_INT (1 << 0)
#define IER_REG_INT        (3 << 2)
#define IER_REG_VALUE      (IER_REG_EN_REC_INT | IER_REG_INT)

#define IIR_REG_REC_NON_EMPTY (2 << 1)

typedef struct {
    reg32 io;
    reg32 ier;
    reg32 iir;
    reg32 lcr;
    reg32 mcr;
    reg32 lsr;
    reg32 msr;
    reg32 scratch;
    reg32 control;
    reg32 status;
    reg32 baud;
} MURegisters;

MURegisters *REGS_MU;

static qword muart_seek(qword n) {
    return 0;
}

static void muart_send(char c) {
    while(!(REGS_MU->lsr & 0x20));

    REGS_MU->io = c;
}

static char muart_read_char() {
    while(!(REGS_MU->lsr & 1));

    return REGS_MU->io & 0xFF;
}

static int muart_write(byte *bytes, int size) {
    for (int i=0; i<size; i++) {
        muart_send(*bytes++);
    }

    return size;
}

static int muart_read(byte *bytes, int size) {
    for (int i=0; i<size; i++) {

        *bytes++ = muart_read_char();
    }

    return size;
}

static IODevice muart = {
    .seek = muart_seek,
    .read = muart_read,
    .write = muart_write,
    .name = "miniuart"
};

int dev_muart_init() {
    REGS_AUX = (AuxRegisters *)(PBASE + 0x00215000);
    REGS_MU = (MURegisters *)(PBASE + 0x00215040);

    gpio_pin_mode(14, PMAlt5);
    gpio_pin_mode(15, PMAlt5);
    gpio_pin_mode(18, PMOut);

    gpio_enable_pupd();

    REGS_AUX->enables = 1;
    REGS_MU->control = 0;
    REGS_MU->ier = IER_REG_VALUE;
    REGS_MU->lcr = 3; //8 bit mode
    REGS_MU->mcr = 0;
    REGS_MU->baud = 270; //115200
    REGS_MU->control = 3; //reentable tx/rx

    dev_register(&muart);

    muart_write((byte *)"MUART INITIALIZED\r\n", 19);

    return 0;
} 

void handle_uart_irq() {
	while((get32((PBASE+0x00215048)) & IIR_REG_REC_NON_EMPTY) == IIR_REG_REC_NON_EMPTY) {
		muart_send(muart_read_char());
	}
}
