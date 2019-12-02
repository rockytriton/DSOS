#include "usb.h"
#include "mm.h"
#include <log.h>

static qword deviceAddressMap;

UsbDevice *usb_dev_create() {

    UsbDevice *dev = (UsbDevice *)allocPage();
    UsbEndpoint *ep = (UsbEndpoint *)allocPage();
    ep->device = dev;
    ep->type = UETControl;
    ep->nextPid = UPTSetup;

    dev->endpoint = ep;
    dev->address = 0;
    dev->hubAddress = 0;
    dev->hubPortNumber = 1;
    dev->splitTransfer = false;

    for (int i=0; i<MAX_FUNCTIONS; i++) {
        dev->functions[i] = 0;
    }

    return dev;
}

bool usb_dev_init(UsbDevice *dev) {
    if (usb_host_get_descriptor(dev, UFDescriptorDevice, UFDescriptorIndexDefault, UFMinMaxPacketSize) != UFMinMaxPacketSize) {
        log_println("Can't get min device descriptor");
        return false;
    }

    if (dev->descriptor.length != sizeof(UsbDeviceDescriptor) || dev->descriptor.type != UFDescriptorDevice) {
        log_println("Invalid device descriptor");
        return false;
    }

    if (!usb_endpoint_set_max_packet(dev->endpoint, dev->descriptor.maxPacketSize)) {
        log_println("Can't set max packet size");
        return false;
    }

    if (usb_host_get_descriptor(dev, UFDescriptorDevice, UFDescriptorIndexDefault, UFMinMaxPacketSize) != sizeof(UsbDeviceDescriptor)) {
        log_println("Can't get device descriptor");
        return false;
    }

    log_println("DEVICE DESCRIPTOR: ");
    log_dump(&dev->descriptor, sizeof(UsbDeviceDescriptor), 16);

    int addr = 0;
    for (; addr <= USB_MAX_ADDRESS; addr++) {
        if (!(deviceAddressMap & ((qword)1 << addr))) {
            break;
        }
    }

    if (addr > USB_MAX_ADDRESS) {
        log_println("Too many devices");
        return false;
    }

    deviceAddressMap |= (qword)1 << addr;

    log_println("Created device at %2X", addr);

    return true;
}

bool usb_endpoint_set_max_packet(UsbEndpoint *ep, dword maxPacketSize) {
    ep->maxPacketSize = maxPacketSize;
    return true;
}

bool usb_request_init(UsbRequest *req, UsbEndpoint *ep, void *buffer, dword bufferLength, UsbSetupData *setupData) {
    req->endpoint = ep;
    req->setupData = setupData;
    req->buffer = buffer;
    req->bufferLength = bufferLength;
    req->status = 0;
    req->resultLength = 0;
    req->callback = NULL;
    req->callbackParam = NULL;
    req->callbackContext = NULL;
    req->completeOnNAK = false;
}


UsbPidType usb_endpoint_next_pid(UsbEndpoint *ep, bool statusStage) {
    if (statusStage) {
        return UPTData1;
    }

    return ep->nextPid;
}
