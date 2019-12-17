#include "spi.h"
#include "../gpio/gpio.h"
#include <sys/timer.h>
#include <log.h>
#include <peripherals/base.h>
#include "utils.h"
#include <sys/irq.h>

typedef void (*IRQ_HANDLER)(SysIrq, void *);

void spi_handle_irq(SysIrq irq, void *p) {
    log_println("HANDLED IRQ: %d", irq);
}

void spi_init_master() {
    log_println("Initializing SPI Master: %8X", PBASE);

    log_println("TESTING TIMER TICKS");
    timer_delay(2000);

    irq_set_handler(ARM_IRQLOCAL0_CNTPNS, spi_handle_irq, 0);

    qword nCNTFRQ;
	asm volatile ("mrs %0, CNTFRQ_EL0" : "=r" (nCNTFRQ));
	assert (nCNTFRQ % 100 == 0, "BAD HZ");
	qword m_nClockTicksPerHZTick = nCNTFRQ / 100;

	qword nCNTPCT;
	asm volatile ("mrs %0, CNTPCT_EL0" : "=r" (nCNTPCT));
	asm volatile ("msr CNTP_CVAL_EL0, %0" :: "r" (nCNTPCT + m_nClockTicksPerHZTick));

	asm volatile ("msr CNTP_CTL_EL0, %0" :: "r" (1));

    log_println("VPT: %X", nCNTPCT);
    log_println("VPT: %X", m_nClockTicksPerHZTick);

    qword pt = clock_physical_ticks();

    log_println("PHYSICAL TICKS: %X", pt);
    clock_init();
    timer_delay(2000);

    qword tick1 = clock_get_ticks();
    timer_delay(1000);
    qword tick2 = clock_get_ticks();

    log_println("TICKS %d - %d = %d", tick2, tick1, tick2 - tick1);
    timer_delay(2000);

    tick1 = clock_get_ticks();
    timer_delay(1000);
    tick2 = clock_get_ticks();

    log_println("TICKS %d - %d = %d", tick2, tick1, tick2 - tick1);
    timer_delay(2000);

    tick1 = clock_get_ticks();
    timer_delay(1000);
    tick2 = clock_get_ticks();

    log_println("TICKS %d - %d = %d", tick2, tick1, tick2 - tick1);
    timer_delay(2000);

    tick1 = clock_get_ticks();
    timer_delay_us(1000);
    tick2 = clock_get_ticks();

    log_println("TICKSU %d - %d = %d", tick2, tick1, tick2 - tick1);
    timer_delay(2000);

    tick1 = clock_get_ticks();
    timer_delay_us(1000);
    tick2 = clock_get_ticks();

    log_println("TICKSU %d - %d = %d", tick2, tick1, tick2 - tick1);
    timer_delay(2000);

    tick1 = clock_get_ticks();
    timer_delay_ms(1);
    tick2 = clock_get_ticks();

    log_println("TICKSM %d - %d = %d", tick2, tick1, tick2 - tick1);
    timer_delay(2000);

    tick1 = clock_get_ticks();
    timer_delay_ms(1);
    tick2 = clock_get_ticks();

    log_println("TICKSM %d - %d = %d", tick2, tick1, tick2 - tick1);
    timer_delay(2000);

    tick1 = clock_get_ticks();
    timer_delay(1000);
    tick2 = clock_get_ticks();

    log_println("TICKS %d - %d = %d", tick2, tick1, tick2 - tick1);
    timer_delay(2000);

    REGS_AUX = (AuxRegisters *)(PBASE + 0x00215000);
    SPI = (SpiRegisters *)(PBASE + 0x204000);

    log_println("SPI: %8X - %8X", SPI, AUX());

    log_println("Setting enable flag: %8X", AUX()->enables);

    AUX()->enables |= (1 << 1);

    log_println("Init 1");

    gpio_pin_mode(9, PMAlt0);
    gpio_pin_mode(8, PMAlt0); //CD
    gpio_pin_mode(7, PMAlt0);
    gpio_pin_mode(10, PMAlt0);
    gpio_pin_mode(11, PMAlt0);
    gpio_pin_mode(24, PMOut); //RS (DC?)
    gpio_pin_mode(25, PMOut); //Reset

    log_println("Init 2");

    SPI->controlStatus = 0x30;
    SPI->clock = 4;

    log_println("Init 3");

    //SPI->controlStatus |= (1 << 22);

    log_println("Init 4");

    timer_delay(500);

    log_println("SPI Master Initialized 2");
}

dword sent = 0;
byte spi_recv_byte();


void spi_reset() {
    gpio_write(25, 1);
    timer_delay(10);
    gpio_write(25, 0);
    timer_delay(1);
    gpio_write(25, 1);
    timer_delay(120);

    //activate CS
    gpio_write(8, 0);
}

void spi_command() {
    gpio_write(24, 0);
    //put32(((dword)(qword)SPI) + 0x28, 1 << 25);
}

void spi_data() {
    gpio_write(24, 1);
    //put32(((dword)(qword)SPI) + 0x1C, 1 << 25);
}

void spi_send_dma(word *data, int size) {
    SPI->controlStatus = 0x40003; // |= 0xB0; //(1 << 7);
    SPI->clock = 4;
    SPI->controlStatus = 0x40183;
}

void spi_start_transaction() {
    SPI->controlStatus = 0x40003; // |= 0xB0; //(1 << 7);
    SPI->clock = 4;
    SPI->controlStatus = 0x40083;
}

void spi_end_transaction() {
    SPI->controlStatus = 0x50033;
    SPI->dataLength = 0;
}

void spi_send_byte(byte b) {

    while(!(SPI->controlStatus & (1 << 18)));

    SPI->fifo = b;

    while(!(get32(((dword)(qword)SPI)) & (1 << 16)));

    sent++;

    //if ((sent % 1000) == 0) {
     //   log_println("SENT: %d", sent);
    //}

    spi_recv_byte();
}

byte spi_recv_byte() {
    while(!(SPI->controlStatus & (1 << 17)));
    byte b = SPI->fifo;
    //log_println("Read back %2X", b);

    return b;
}

void spi_activate_transfer() {
    SPI->controlStatus |= (1 << 7);
}

void spi_send_command(byte command, byte length, byte *data) {
    log_println("Sending SPI Command: %d", command);

    spi_send_byte(command);
    spi_recv_byte();
    spi_send_byte(0xff);
    byte resp = spi_recv_byte();

    assert(resp, "failed to send command");

    spi_send_byte(length);
    spi_recv_byte();

    log_println("Finished Writing Length");

    for (int i=0; i<length; i++) {
        spi_send_byte(data[i]);
    }

    log_println("Finished Writing Data");

    
    //SPI->fifo = (command >> 16) & 0xFF;
    //SPI->fifo = (command >> 8) & 0xFF;
    //SPI->fifo = (command >> 0) & 0xFF;

}