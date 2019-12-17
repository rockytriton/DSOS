#pragma once

#include <common.h>

typedef struct {
    reg32 controlStatus;
    reg32 fifo;
    reg32 clock;
    reg32 dataLength;
    reg32 lossi;
    reg32 dmaControls;
} SpiRegisters;

static SpiRegisters *SPI;

void spi_init_master();
void spi_send_command(byte command, byte length, byte *data);
void spi_send_byte(byte b);
void spi_activate_transfer();
void spi_command();
void spi_data();
void spi_reset();
void spi_start_transaction();
void spi_end_transaction();
void spi_send_dma(word *data, int size);

void spimaster_init();
void spimaster_write(byte chipSelect, byte *buffer, int count);
