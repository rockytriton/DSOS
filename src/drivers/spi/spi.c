#include "spi.h"
#include "../gpio/gpio.h"
#include <sys/timer.h>
#include <log.h>
#include <peripherals/base.h>
#include "utils.h"

void spi_init_master() {
    log_println("Initializing SPI Master: %8X", PBASE);

    REGS_AUX = (AuxRegisters *)(PBASE + 0x00215000);
    SPI = (SpiRegisters *)(PBASE + 0x204000);

    log_println("SPI: %8X - %8X", SPI, AUX());

    log_println("Setting enable flag: %8X", AUX()->enables);

    AUX()->enables |= (1 << 1);

    log_println("Init 1");

    gpio_pin_mode(9, PMAlt0);
    gpio_pin_mode(8, PMOut); //CD
    gpio_pin_mode(7, PMAlt0);
    gpio_pin_mode(10, PMAlt0);
    gpio_pin_mode(11, PMAlt0);
    gpio_pin_mode(24, PMOut); //RS (DC?)
    gpio_pin_mode(25, PMOut); //Reset

    log_println("Init 2");

    SPI->controlStatus = 0x30;
    SPI->clock = 4;

    log_println("Init 3");

    SPI->controlStatus |= (1 << 22);

    log_println("Init 4");

    timer_delay(500);

    log_println("SPI Master Initialized 2");
}

dword sent = 0;
byte spi_recv_byte();

void spi_send_byte(byte b) {
    SPI->controlStatus |= 0xB0; //(1 << 7);
    //log_println("WAITING TO SEND");
    while(!(SPI->controlStatus & (1 << 18)));
    //log_println("Sending a SPI Byte: %2X", b);
    put32(((dword)(qword)SPI) + 4, b);
    //SPI->fifo = b;
    //log_println("Sent %2X", b);
    while(!(get32(((dword)(qword)SPI)) & (1 << 16)));
    //log_println("Done", b);
    spi_recv_byte();

    SPI->controlStatus &= ~0xB0; //(1 << 7);

    sent++;

    //if ((sent % 1000) == 0) {
     //   log_println("SENT: %d", sent);
    //}
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