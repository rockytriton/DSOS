#pragma once

#include <common.h>

typedef struct {
    dword transferInfo;
    dword sourceAddress;
    dword destDescress;
    dword transferLength;
    dword mode2dStride;
    dword nextBlockAddress;
    dword res[2];
} DmaControlBlock;

typedef enum {
    DCNone = -1,
    DCNormal = 0x81
} DmaChannelType;

typedef enum {
    DRSourceNone = 0,
    DRSourcePWM1 = 1,
    DRSourcePcmTx = 2,
    DRSourcePcmRx = 3,
    DRSourcePwm = 5,
    DRSourceSpiTx = 6,
    DRSourceSpiRx = 7,
    DRSourceEmmc = 11,
    DRSourceUartTx = 12,
    DRSourceUartRx = 14
} DmaRequest;

typedef void DmaCompletionCallback (dword channel, bool status, void *param);

typedef struct {
    dword channel;
    byte *controlBlockBuffer;
    DmaControlBlock *block;
    bool irq;
    DmaCompletionCallback *callback;
    void *callbackParam;
    bool status;
    dword *destAddress;
    dword bufferLength;
} DmaChannel;

DmaChannel *dma_open_channel(dword channel);
void dma_close_channel(DmaChannel *channel);

void dma_setup_mem_copy(DmaChannel *channel, void *dest, void *src, dword length, dword burstLen, bool cached);
void dma_setup_mem_copy2d(DmaChannel *channel, void *dest, void *src, dword length, dword blockCount, dword blockStride, dword burstLen);

void dma_setup_io_read(DmaChannel *channel, void *dest, dword ioAddress, dword length, DmaRequest request);
void dma_setup_io_write(DmaChannel *channel, dword ioAddress, void *dest, dword length, DmaRequest request);

void dma_start(DmaChannel *channel);
bool dma_wait(DmaChannel *channel);
