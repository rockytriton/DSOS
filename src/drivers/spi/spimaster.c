#include "spi.h"
#include "../gpio/gpio.h"
#include <sys/timer.h>
#include <log.h>
#include <peripherals/base.h>
#include "utils.h"
#include "mm.h"

#define CS_LEN_LONG	(1 << 25)
#define CS_DMA_LEN	(1 << 24)
#define CS_CSPOL2	(1 << 23)
#define CS_CSPOL1	(1 << 22)
#define CS_CSPOL0	(1 << 21)
#define CS_RXF		(1 << 20)
#define CS_RXR		(1 << 19)
#define CS_TXD		(1 << 18)
#define CS_RXD		(1 << 17)
#define CS_DONE		(1 << 16)
#define CS_LEN		(1 << 13)
#define CS_REN		(1 << 12)
#define CS_ADCS		(1 << 11)
#define CS_INTR		(1 << 10)
#define CS_INTD		(1 << 9)
#define CS_DMAEN	(1 << 8)
#define CS_TA		(1 << 7)
#define CS_CSPOL	(1 << 6)
#define CS_CLEAR_RX	(1 << 5)
#define CS_CLEAR_TX	(1 << 4)
#define CS_CPOL__SHIFT	3
#define CS_CPHA__SHIFT	2
#define CS_CS		(1 << 0)
#define CS_CS__SHIFT	0

#define SPI0_CS (PBASE + 0x204000)
#define SPI0_CLOCK (SPI0_CS + 0x08)

#define SPI0_FIFO		(SPI0_CS + 0x04)
#define SPI0_CLK		(SPI0_CS + 0x08)
#define SPI0_DLEN		(SPI0_CS + 0x0C)
#define SPI0_LTOH		(SPI0_CS + 0x10)
#define SPI0_DC		(SPI0_CS + 0x14)

void spimaster_init() {
    gpio_pin_mode(9, PMAlt0);
    gpio_pin_mode(8, PMAlt0); //CD
    gpio_pin_mode(7, PMAlt0);
    gpio_pin_mode(10, PMAlt0);
    gpio_pin_mode(11, PMAlt0);

    writeMmio(SPI0_CLOCK, 4);
    writeMmio(SPI0_CS, 0);
}

void spimaster_write(byte chipSelect, byte *buffer, int count) {

    writeMmio(SPI0_DLEN, count);
    writeMmio(SPI0_CS, (readMmio (SPI0_CS) & ~CS_CS)
			      | (chipSelect << CS_CS__SHIFT)
			      | CS_CLEAR_RX | CS_CLEAR_TX
			      | CS_TA);

    int writeCount = 0;
    int readCount = 0;

    while(writeCount < count || readCount < count) {
        while(writeCount < count && (readMmio(SPI0_CS) & CS_TXD)) {
            dword data = *buffer++;
            writeMmio(SPI0_FIFO, data);
            writeCount++;
        }

        while(readCount < count && (readMmio(SPI0_CS) & CS_RXD)) {
            reg32 data = readMmio(SPI0_FIFO);
            readCount++;
        }
    }

    while(!(readMmio(SPI0_CS) & CS_DONE)) {
        while(readMmio(SPI0_CS) & CS_RXD) {
            readMmio(SPI0_FIFO);
        }
    }

    writeMmio(SPI0_CS, readMmio(SPI0_CS) & ~CS_TA);
}