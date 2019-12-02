#include "dwhci.h"
#include <sys/util.h>
#include <log.h>
#include "transfer_stage.h"
#include "mm.h"

#define MAX_WAIT_BLOCKS 16

static volatile dword waitBlocks = 0;
static volatile dword channelAllocations = 0;
static volatile bool waiting[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

static byte allocate_wait_block() {
    for (byte n = 0; n < MAX_WAIT_BLOCKS; n++) {
        if (!flag32_set(waitBlocks, n)) {
            waitBlocks |= (1 << n);
            return n;
        }
    }

    return 0xFF;
}

static void free_wait_block(byte n) {
    waitBlocks &= ~(1 << n);
}

void dwhci_transfer_callback(UsbRequest *req, void *p, void *context) {
    UsbDevice *dev = (UsbDevice *)context;
    dword waitBlock = (dword)(qword)p;
}

dword allocate_channel() {
    for (byte i=0; i<usbChannels; i++) {
        if (!flag32_set(channelAllocations, i)) {
            channelAllocations |= (1 << i);
            return i;
        }
    }

    return 0xFF;
}


static bool dwhci_transfer_async(UsbRequest *req, bool in, bool statusStage, dword timeout) {
    dword channel = allocate_channel();

    if (channel == 0xFF) {
        log_println("FAILED TO ALLOC CHANNEL");
        return false;
    }

    TransferStageData *sd = (TransferStageData *)allocPage();
    transfer_stage_init(sd, channel, req, in, statusStage, timeout);
    req->endpoint->device->stageData[channel] = sd;

    USB_HOST->allChanIntMask |= (1 << channel);

    if (sd->splitTransaction) {
        sd->state = SSNoSplit;
    } else {
        //TODO if not true?

        sd->state = SSStartSplit;
        sd->splitComplete = false;
        //TODO Frame Scheduler
    }
}

bool dwhci_transer_stage(UsbRequest *req, bool in, bool statusStage, dword timeout) {
    byte waitBlock = allocate_wait_block();

    if (waitBlock == 0xFF) {
        log_println("Failed to wait block");
        return false;
    }

    req->callbackContext = dwhci_transfer_callback;
    req->callbackParam = waitBlock;
    req->callbackContext = req->endpoint->device;
    waiting[waitBlock] = true;

}

bool dwhci_request_sync(UsbRequest *req, dword timeout) {
    req->status = 0;

    if (req->endpoint->type == UETControl) {
        if (req->setupData->requestType == URTIn) {

        }
    }
}
