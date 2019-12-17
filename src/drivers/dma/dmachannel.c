#include "dmachannel.h"
#include "mm.h"
#include "log.h"
#include <sys/timer.h>


#define GPU_CACHED_BASE		0x40000000
#define GPU_UNCACHED_BASE	0xC0000000

#define GPU_MEM_BASE	GPU_UNCACHED_BASE


// Convert ARM address to GPU bus address (does also work for aliases)
#define BUS_ADDRESS(addr)	(((addr) & ~0xC0000000) | GPU_MEM_BASE)

#define ARM_DMA_BASE (PBASE + 0x7000)
#define ARM_DMA_INT_STATUS		(ARM_DMA_BASE + 0xFE0)
#define ARM_DMA_ENABLE			(ARM_DMA_BASE + 0xFF0)
#define ARM_DMACHAN_CS(chan)		(ARM_DMA_BASE + ((chan) * 0x100) + 0x00)

#define ARM_DMACHAN_SOURCE_AD(chan)	(ARM_DMA_BASE + ((chan) * 0x100) + 0x0C)
#define ARM_DMACHAN_DEST_AD(chan)	(ARM_DMA_BASE + ((chan) * 0x100) + 0x10)
#define ARM_DMACHAN_TXFR_LEN(chan)	(ARM_DMA_BASE + ((chan) * 0x100) + 0x14)

#define TXFR_LEN_MAX			0x3FFFFFFF
#define TXFR_LEN_MAX_LITE		0xFFFF

#define TI_PERMAP_SHIFT			16
#define TI_BURST_LENGTH_SHIFT		12
#define DEFAULT_BURST_LENGTH		0
#define TI_SRC_IGNORE			(1 << 11)
#define TI_SRC_DREQ			(1 << 10)
#define TI_SRC_WIDTH			(1 << 9)
#define TI_SRC_INC			(1 << 8)
#define TI_DEST_DREQ			(1 << 6)
#define TI_DEST_WIDTH			(1 << 5)
#define TI_DEST_INC			(1 << 4)
#define TI_WAIT_RESP			(1 << 3)
#define TI_TDMODE			(1 << 1)
#define TI_INTEN			(1 << 0)

#define DEBUG_LITE (1 << 28)

#define CS_RESET			(1 << 31)
#define CS_ABORT			(1 << 30)
#define CS_WAIT_FOR_OUTSTANDING_WRITES	(1 << 28)
#define CS_PANIC_PRIORITY_SHIFT		20
    #define DEFAULT_PANIC_PRIORITY		15
#define CS_PRIORITY_SHIFT		16
    #define DEFAULT_PRIORITY		1
#define CS_ERROR			(1 << 8)
#define CS_INT				(1 << 2)
#define CS_END				(1 << 1)
#define CS_ACTIVE			(1 << 0)

#define ARM_DMACHAN_CONBLK_AD(chan)	(ARM_DMA_BASE + ((chan) * 0x100) + 0x04)
#define ARM_DMACHAN_NEXTCONBK(chan)	(ARM_DMA_BASE + ((chan) * 0x100) + 0x1C)
#define ARM_DMACHAN_DEBUG(chan)		(ARM_DMA_BASE + ((chan) * 0x100) + 0x20)

static word channelMap = 0x1F35;

static int16_t allocate_channel(dword channel) {
    if (!(channel & ~0x0F)) {
        log_println("DMA: Explicit Allocate");

        if (channelMap & (1 << channel)) {
            channelMap &= ~(1 << channel);
            return channel;
        }

        return -1;
    } else {
        log_println("DMA: Arbitrary Allocate");

        int16_t i = channel == DCNormal ? 6 : 12;

        for (; i > 0; i--) {
            if (channelMap & (1 << i)) {
                channelMap &= ~(1 << i);

                return i;
            }
        }
    }

    return DCNone;
}

DmaChannel *dma_open_channel(dword channel) {

    DmaChannel *dma = (DmaChannel *)allocPage();
    dma->channel = allocate_channel(channel);

    assert(dma->channel != DCNone, "Invalid DMA Channel");

    dma->controlBlockBuffer = allocPage(); //(byte *)(((dword)dma) + sizeof(DmaChannel));
    dma->block = (DmaControlBlock *)((((dword)dma->controlBlockBuffer) + 31) & ~31);
    dma->block->res[0] = 0;
    dma->block->res[1] = 0;
    
    writeMmio(ARM_DMA_ENABLE, readMmio(ARM_DMA_ENABLE) | (1 << dma->channel));
    timer_delay(3);
    writeMmio(ARM_DMACHAN_CS(dma->channel), CS_RESET);

    while(readMmio(ARM_DMACHAN_CS(dma->channel)) & CS_RESET);

    log_println("DMA Enabled for channel %d", dma->channel);

    return dma;
}

void dma_close_channel(DmaChannel *channel) {

}

#define DATA_CACHE_LINE_LENGTH_MIN	64
#define DataSyncBarrier()	asm volatile ("dsb sy" ::: "memory")
#define DataMemBarrier() 	asm volatile ("dmb sy" ::: "memory")


void CleanAndInvalidateDataCacheRange (qword nAddress, qword nLength)
{
	nLength += DATA_CACHE_LINE_LENGTH_MIN;

	while (1)
	{
		asm volatile ("dc civac, %0" : : "r" (nAddress) : "memory");

		if (nLength < DATA_CACHE_LINE_LENGTH_MIN)
		{
			break;
		}

		nAddress += DATA_CACHE_LINE_LENGTH_MIN;
		nLength  -= DATA_CACHE_LINE_LENGTH_MIN;
	}

	DataSyncBarrier ();
}


void dma_setup_mem_copy(DmaChannel *channel, void *dest, void *src, dword length, dword burstLen, bool cached) {
    assert(length <= TXFR_LEN_MAX, "TXFR_LEN_MAX");
    assert(!(readMmio (ARM_DMACHAN_DEBUG (channel->channel)) & DEBUG_LITE) || length <= TXFR_LEN_MAX_LITE, "ARM_DMACHAN_DEBUG");

    channel->block->transferInfo = (burstLen << TI_BURST_LENGTH_SHIFT)
						    | TI_SRC_WIDTH
						    | TI_SRC_INC
						    | TI_DEST_WIDTH
						    | TI_DEST_INC;

    channel->block->sourceAddress = ((dword)src);
    channel->block->destDescress = ((dword)dest);
    channel->block->transferLength = length;
    channel->block->mode2dStride = 0;
    channel->block->nextBlockAddress = 0;

    if (cached) {
        channel->destAddress = (dword)dest;
        channel->bufferLength = length;

        //TODO...
    } else {
        channel->destAddress = 0;
    }
}

void dma_setup_mem_copy2d(DmaChannel *channel, void *dest, void *src, dword length, dword blockCount, dword blockStride, dword burstLen) {
    
}


void dma_setup_io_read(DmaChannel *channel, void *dest, dword ioAddress, dword length, DmaRequest request) {
    
}

void dma_setup_io_write(DmaChannel *channel, dword ioAddress, void *dest, dword length, DmaRequest request) {
    
}


void dma_start(DmaChannel *channel) {
	assert (!(readMmio (ARM_DMACHAN_CS (channel->channel)) & CS_INT), "DMA START 1");
	assert (!(readMmio (ARM_DMA_INT_STATUS) & (1 << channel->channel)), "DMA START 2");


	writeMmio (ARM_DMACHAN_CONBLK_AD (channel->channel), BUS_ADDRESS ((dword) channel->block));

	CleanAndInvalidateDataCacheRange ((dword) channel->block, sizeof(DmaControlBlock));

	writeMmio (ARM_DMACHAN_CS (channel->channel),   CS_WAIT_FOR_OUTSTANDING_WRITES
					      | (DEFAULT_PANIC_PRIORITY << CS_PANIC_PRIORITY_SHIFT)
					      | (DEFAULT_PRIORITY << CS_PRIORITY_SHIFT)
					      | CS_ACTIVE);
}

bool dma_wait(DmaChannel *channel) {
    dword cs;
	while ((cs = readMmio (ARM_DMACHAN_CS (channel->channel))) & CS_ACTIVE)
	{
		// do nothing
	}

	channel->status = cs & CS_ERROR ? false : true;

    if (channel->destAddress != 0) {
        log_println("Clearing and invalidating");
        CleanAndInvalidateDataCacheRange (channel->destAddress, channel->bufferLength);
    }

    return channel->status;
}


