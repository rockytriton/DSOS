#include "transfer_stage.h"
#include "mm.h"
#include <sys/timer.h>

#define HZ		100			// ticks per second

#define MSEC2HZ(msec)	((msec) * HZ / 1000)

void transfer_stage_init(TransferStageData *data, byte channel, UsbRequest *request, bool in, bool statusStage, dword timeoutMs) {
    data->device = request->endpoint->device;
    data->channel = channel;
    data->request = request;
    data->in = in;
    data->statusStage = statusStage;
    data->timeoutHz = 0;
    data->splitComplete = false;
    data->totalBytesTransfered = 0;
    data->state = 0;
    data->subState = 0;
    data->transactionStatus = 0;
    data->tempBuffer = 0;
    data->startTicksHz = 0;
    data->frameScheduler = 0;

    data->speed = request->endpoint->device->speed;
    data->maxPacketSize = request->endpoint->maxPacketSize;
    data->splitTransaction = request->endpoint->device->splitTransfer;

    if (!statusStage) {
        if (usb_endpoint_next_pid(request->endpoint, statusStage) == UPTSetup) {
            data->buffer = request->setupData;
            data->transferSize = sizeof(UsbSetupData);
        } else {
            data->buffer = request->buffer;
            data->transferSize = request->bufferLength;
        }

        data->packets = (data->transferSize + data->maxPacketSize - 1) / data->maxPacketSize;

        if (data->splitTransaction) {
            if (data->transferSize > data->maxPacketSize) {
                data->bytesPerTransaction = data->maxPacketSize;
            } else {
                data->bytesPerTransaction = data->transferSize;
            }

            data->packetsPerTransaction = 1;
        } else {
            data->bytesPerTransaction = data->transferSize;
            data->packetsPerTransaction = data->packets;
        }
    } else {
        data->tempBuffer = allocPage();
        data->buffer = data->tempBuffer;
        data->transferSize = 0;
        data->bytesPerTransaction;
        data->packets = 1;
        data->packetsPerTransaction = 1;
    }

    if (data->splitTransaction) {
        bool periodic = request->endpoint->type == UETInterrupt || request->endpoint->type == UETIso;

        if (periodic) {
            //TODO..
        } else {
            //TODO
        }
    } else {
        if (request->endpoint->device->hubAddress == 0 && data->speed != USHigh) {
            //TODO
        }
    }

    if (timeoutMs != 0) {
        data->timeoutHz = MSEC2HZ(timeoutMs);
        data->startTicksHz = timer_ticks();
    }
}

