#pragma once

#include "usb.h"

typedef struct {

} UsbHostController;

typedef struct {

} DwHciDevice;

UsbSpeed usb_port_speed();

int usb_host_get_descriptor(UsbDevice *dev, UsbFlags deviceType, UsbFlags descIndex, dword size);

int usb_host_control_message(UsbEndpoint *ep, byte requestType, byte request, word value, word index, void *data, word dataSize);

