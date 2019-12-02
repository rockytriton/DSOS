#include "hostcontroller.h"
#include "peripherals/base.h"
#include <log.h>
#include <mailbox/mailbox.h>
#include <sys/irq.h>
#include <sys/util.h>
#include "dwhci.h"
#include "dwhci_root.h"

void usb_irq(SysIrq irq, void *p) {
    log_println("USB INTERRUPT");
}

bool usb_reset() {
    if (!registerWait(&USB_CORE->reset, UCRAhbIdle, true, 5000)) {
        log_println("Failed to wait for reset");
        return false;
    }

    reg32 n = USB_CORE->reset;
    n |= UCRSoftReset;
    USB_CORE->reset = n;

    if (!registerWait(&USB_CORE->reset, UCRSoftReset, false, 5000)) {
        log_println("Failed to wait after reset: %X", USB_CORE->reset);
        return false;
    }

    timer_delay(100);

    return true;
}

bool usb_core_init() {
    reg32 n = USB_CORE->usbConfig;
    n &= ~UCExtVbusDrv;
    n &= ~UCTermSelDlPulse;
    USB_CORE->usbConfig = n;

    if (!usb_reset()) {
        log_println("Failed to reset");
        return false;
    }

    log_println("Reset USB");

    n = USB_CORE->usbConfig;
    n &= ~UCUtmiSel;
    n &= ~UCPhyIf;
    USB_CORE->usbConfig = n;

    log_println("Configured USB: %8X", USB_CORE->hwConfig2);

    assert(((USB_CORE->hwConfig2) >> 3) & 3, "Invalid hw config value");

    n = USB_CORE->usbConfig;
    dword hsPhyType = ((USB_CORE->hwConfig2 >> 6) & 3);
    dword fsPhyType = ((USB_CORE->hwConfig2 >> 8) & 3);

    if (hsPhyType == 2 && fsPhyType == 1) {
        n |= UCFsls;
        n |= UCClockSusM;
        log_println("ULPI Dedicated");
    } else {
        n &= ~UCFsls;
        n &= ~UCClockSusM;
        log_println("!ULPI Dedicated");
    }
    USB_CORE->usbConfig = n;

    usbChannels = ((USB_CORE->hwConfig2 >> 14) & 0xF) + 1;

    log_println("USB Channels: %d", usbChannels);

    n = USB_CORE->ahbConfig;
    n |= AHBCDmaEnable | AHBCWaitAxiWrites;
    n &= ~AHBCMaxAxiBurstMask;
    USB_CORE->ahbConfig = n;

    n = USB_CORE->usbConfig;
    n &= ~UCHnpCap;
    n &= ~UCSrpCap;
    USB_CORE->usbConfig = n;

    //enable all interrupts:
    USB_CORE->intStat = 0xFFFFFFFF;

    log_println("USB Core Initialized");

    return true;
}

void flushTx(dword num) {
    reg32 n = UCRTxFifoFlush;
    n &= ~UCRTxFifoMask;
    n |= (num << 6);
    USB_CORE->reset = n;

    registerWait(&USB_CORE->reset, UCRTxFifoFlush, false, 10);

    timer_delay(1);
}

void flushRx() {
    USB_CORE->reset = UCRRxFifoFlush;

    registerWait(&USB_CORE->reset, UCRRxFifoFlush, false, 10);

    timer_delay(1);
}

bool usb_init_host() {
    reg32 *power = (USB_HOST_BASE + 0xE00);
    *power = 0;

    reg32 hc = USB_HOST->hostConfig;
    hc &= ~0x3; //clear pclk sel mask

    reg32 c2 = USB_CORE->hwConfig2;
    reg32 uc = USB_CORE->usbConfig;
    dword hsPhyType = ((USB_CORE->hwConfig2 >> 6) & 3);
    dword fsPhyType = ((USB_CORE->hwConfig2 >> 8) & 3);

    if (hsPhyType == 2 && fsPhyType == 1 && uc & UCFsls) {
        hc |= 1; //48mhz
        log_println("Host 48 Mhz");
    } else {
        //30_60 mhz
        log_println("Host 30-60 Mhz");
    }

    USB_HOST->hostConfig = hc;

    flushTx(0x10);
    flushRx();

    log_println("FIFOs flushed");

    reg32 hp = USB_HOST->hostPort;
    hp &= ~UHPDefaultMask; //default mask

    if (!(hp & (1 << 12))) {
        hp |= (1 << 12);

        log_println("Powering On");
        USB_HOST->hostPort = hp;
    }

    //clear pending
    USB_CORE->intMask = 0;
    
    //enable all interrupts:
    USB_CORE->intStat = 0xFFFFFFFF;

    USB_CORE->intStat |= (1 << 25);

    log_println("Host interrupts enabled");

    return true;
}

UsbSpeed usb_port_speed() {
    switch((USB_HOST->hostPort >> 17 ) & 0x3) {
        case 0: return USHigh;
        case 1: return USFull;
        case 2: return USLow;
    }

    return USUnknown;
}

static void usb_rescan() {
    if (!rootPortEnabled) {

        if (dwhci_root_enable()) {
            rootPortEnabled = true;
            
            if (!dwhci_root_init()) {
                log_println("Failed to init root port");
            } else {
                log_println("Root Port Initialized");
            }
        }
    }
}

void dev_init_usb() {
    USB_CORE = (UsbCoreRegisters *)USB_BASE;
    USB_HOST = (UsbHostRegisters *)USB_HOST_BASE;

    log_println("Initializing USB");
    log_println("USB Vendor ID: %X", USB_CORE->vendorId);

    assert(USB_CORE->vendorId == 0x4F54280A, "Invalid Vendor ID");

    assert(mailbox_power(DPTUsb, true), "Failed to start USB");

    log_println("Started up USB");

    reg32 ac = USB_CORE->ahbConfig;
    ac &= ~AHBCGlobalIntMask;
    USB_CORE->ahbConfig = ac;

    irq_set_handler(SIUsb, usb_irq, 0);

    assert(usb_core_init(), "Failed to Init USB CORE");

    //enable global interrupts
    USB_CORE->ahbConfig |= 1;

    assert(usb_init_host(), "Failed to Init USB HOST");
    
    usb_rescan();
}

int usb_host_get_descriptor(UsbDevice *dev, UsbFlags deviceType, UsbFlags descIndex, dword size) {
    return -1;
}

int usb_host_control_message(UsbEndpoint *ep, byte requestType, byte requestCode, word value, word index, void *data, word dataSize) {
    UsbSetupData setup;

    setup.requestType = requestType;
    setup.request = requestCode;
    setup.value = value;
    setup.index = index;
    setup.length = dataSize;

    UsbRequest request;
    usb_request_init(&request, ep, data, dataSize, &setup);

    return -1;
}

