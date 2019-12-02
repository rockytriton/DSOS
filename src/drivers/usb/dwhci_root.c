#include "dwhci.h"
#include "dwhci_root.h"
#include "hostcontroller.h"
#include "mm.h"
#include <sys/util.h>

static UsbDevice *rootDevice;
static UsbEndpoint *rootEndpoint;

bool dwhci_root_enable() {
    dword delay = 510;
    
    if (!registerWait(&USB_HOST->hostPort, 1, true, delay)) {
        log_println("Unable to enable root port");
        return false;
    }

    timer_delay(100);

    reg32 hp = USB_HOST->hostPort;
    hp &= ~UHPDefaultMask;
    hp |= UHPReset;
    USB_HOST->hostPort = hp;

    timer_delay(50);
    hp = USB_HOST->hostPort;
    hp &= ~UHPDefaultMask;
    hp &= ~UHPReset;
    USB_HOST->hostPort = hp;

    timer_delay(20);

    log_println("Enabled Root Port");
    return true;
}

bool dwhci_root_init() {
    UsbSpeed speed = usb_port_speed();

    if (speed == USUnknown) {
        log_println("Can't detect port speed");
        return false;
    }

    rootDevice = usb_dev_create();
    rootDevice->speed = speed;

    if (!usb_dev_init(rootDevice)) {
        log_println("Failed to init root device");
        return false;
    }

    return true;
}
