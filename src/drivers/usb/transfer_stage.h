#pragma once

#include "usb.h"

typedef struct _TransferStageData {
    byte channel;
    UsbRequest *request;
    bool in;
    bool statusStage;
    dword timeoutHz;
    bool splitTransaction;
    bool splitComplete;
    UsbDevice *device;
    UsbSpeed speed;
    dword maxPacketSize;
    dword transferSize;
    dword packets;
    dword bytesPerTransaction;
    dword packetsPerTransaction;
    dword totalBytesTransfered;
    dword state;
    dword subState;
    dword transactionStatus;
    dword *tempBuffer;
    void *buffer;
    dword startTicksHz;

    void *frameScheduler;

} TransferStageData;

void transfer_stage_init(TransferStageData *data, byte channel, UsbRequest *request, bool in, bool statusStage, dword timeoutMs);
