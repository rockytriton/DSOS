#pragma once

#include <common.h>

#define MAX_FUNCTIONS 10
#define USB_MAX_ADDRESS 63
#define MAX_CHANNELS 16

typedef enum {
    USLow,
    USFull,
    USHigh,
    USSuper,
    USUnknown
} UsbSpeed;

typedef enum {
    UFDescriptorDevice = 1,
    UFDescriptorIndexDefault = 0,
    UFMinMaxPacketSize = 8
} UsbFlags;

typedef struct {
    byte requestType;
    byte request;
    word value;
    word index;
    word length;
} UsbSetupData PACKED;

typedef struct {
    byte length;
    byte type;
    word usbCode;
    byte devClass;
    byte devSubClass;
    byte devProtocol;
    byte maxPacketSize;
    word vendorId;
    word produceId;
    word deviceCode;
    byte manufacturer;
    byte product;
    byte serialNumber;
    byte numConfigs;
} UsbDeviceDescriptor;

typedef struct _UsbEndpoint UsbEndpoint;

typedef struct _TransferStageData TransferStageData;

typedef struct _UsbDevice {
    dword functions[MAX_FUNCTIONS];
    UsbEndpoint *endpoint;
    UsbSpeed speed;
    UsbDeviceDescriptor descriptor;

    void *rootPort;
    void *hub;
    dword hubIndex;
    byte address;
    bool splitTransfer;
    byte hubAddress;
    byte hubPortNumber;
    struct _UsbDevice *hubDevice;

    void *configDescriptor;
    void *configParser;

    //DWHCIDevice
    TransferStageData *stageData[MAX_CHANNELS];
} UsbDevice;

typedef enum {
    UETControl,
    UETBulk,
    UETInterrupt,
    UETIso
} UsbEndpointType;

typedef enum {
    UPTSetup,
    UPTData0,
    UPTData1
} UsbPidType;

typedef struct _UsbEndpoint {
    UsbDevice *device;
    dword maxPacketSize;
    UsbEndpointType type;
    UsbPidType nextPid;
} UsbEndpoint;

typedef struct _UsbRequest UsbRequest;

typedef void (*UsbRequestCallback)(UsbRequest *request, void *param, void *context);

typedef enum {
    URTOut = 0,
    URTIn = 0x80,
    URTClass = 0x20,
    URTVendor = 0x40,
    URTToDevice = 0,
    URTToInterface = 1,
    URTToOther = 3
} UsbRequestType;

typedef enum {
    URCStatus = 0,
    URCClearFeature = 1,
    URCSetFeature = 3,
    URCSetAddress = 5,
    URCGetDescriptor = 6,
    URCSetConfig = 9,
    URCSetInterface = 11
} UsbRequestCode;

typedef enum {
    UDTDevice = 1,
    UDTConfig = 2,
    UDTString = 3,
    UDTInterface = 4,
    UDTEndpoint = 5
} UsbDescType;

typedef struct _UsbRequest {
    UsbEndpoint *endpoint;
    UsbSetupData *setupData;
    void *buffer;
    dword bufferLength;
    int status;
    dword resultLength;
    UsbRequestCallback callback;
    void *callbackParam;
    void *callbackContext;
    bool completeOnNAK;
} UsbRequest;

UsbDevice *usb_dev_create();

bool usb_dev_init(UsbDevice *dev);

bool usb_endpoint_set_max_packet(UsbEndpoint *ep, dword maxPacketSize);

bool usb_request_init(UsbRequest *req, UsbEndpoint *ep, void *buffer, dword bufferLength, UsbSetupData *setupData);

bool usb_request_sync(UsbRequest *req, dword timeout);

UsbPidType usb_endpoint_next_pid(UsbEndpoint *ep, bool statusStage);
