#pragma once

#include "usb.h"

#define USB_BASE (PBASE + 0x980000)
#define USB_HOST_BASE (USB_BASE + 0x400)

typedef struct {
    reg32 otgControl;
    reg32 otgInt;
    reg32 ahbConfig;
    reg32 usbConfig;
    reg32 reset;
    reg32 intStat;
    reg32 intMask;
    reg32 statRead;
    reg32 statPop;
    reg32 rxFifoSize;
    reg32 txFifoSize;
    reg32 txStat;
    reg32 i2cControl;
    reg32 vendorControl;
    reg32 gpio;
    reg32 userId;
    reg32 vendorId;
    reg32 hwConfig1;
    reg32 hwConfig2;
    reg32 hwConfig3;
    reg32 hwConfig4;
    reg32 lpmConfig;
    reg32 powerDown;
    reg32 dfifoConfig;
    reg32 adpControl;
    reg32 res1[7];
    reg32 mdioControl;
    reg32 mdioData;
    reg32 vbusDrive;
    reg32 res2[8];
    reg32 hostPerTxFifoSize;
    
} UsbCoreRegisters;

typedef struct {
    reg32 character;
    reg32 splitControl;
    reg32 interrupt;
    reg32 interruptMask;
    reg32 transferSize;
    reg32 dmaAddress;
    reg32 dmaBuffer;
} UsbHostChannel;

typedef struct {
    reg32 hostConfig;
    reg32 frmInternal;
    reg32 frmNumber;
    reg32 res1;
    reg32 perTxFifoStat;
    reg32 allChanInt;
    reg32 allChanIntMask;
    reg32 frmLastBase;
    reg32 res2[8];
    reg32 hostPort;
    reg32 res3[47];
    UsbHostChannel channels[15];
} UsbHostRegisters;

typedef enum {
    AHBCGlobalIntMask = 1,
    AHBCMaxAxiBurstShift = 1,
    AHBCMaxAxiBurstMask = (3 << 1),
    AHBCWaitAxiWrites = (1 << 4),
    AHBCDmaEnable = (1 << 5), 
    AHBCAhbSingle = (1 << 23)
} CoreAhbConfig;

typedef enum {
    UCPhyIf = 1 << 3,
    UCUtmiSel = 1 << 4,
    UCSrpCap = 1 << 8,
    UCHnpCap = 1 << 9,
    UCFsls = 1 << 17,
    UCClockSusM = 1 << 19,
    UCExtVbusDrv = 1 << 20,
    UCTermSelDlPulse = 1 << 22
} UsbConfig;

typedef enum {
    UCRSoftReset = 1 << 0,
    UCRRxFifoFlush = 1 << 4,
    UCRTxFifoFlush = 1 << 5,
    UCRTxFifoNum = 6,
    UCRTxFifoMask = 0x1F << 6,
    UCRAhbIdle = 1 << 31
} UsbCoreReset;

typedef enum {
    PTNotSupported = 0,
    PTUtmi = 1,
    PTUlpi = 2,
    PTBoth = 3
} UsbConfigPhyType;

typedef enum {
    UHPDefaultMask = ((1 << 1) | (1 << 2) | (1 << 3) | (1 << 5)),
    UHPReset = (1 << 8)
} UsbHostPortType;

typedef enum {
    SSNoSplit,
    SSStartSplit,
    SSCompleteSplit,
    SSPeriodicDelay,
    SSUnknown
} StageState;

typedef enum {
    SSSWaitDisable,
    SSSWaitComplete,
    SSSUnknown
} StageSubState;

static UsbCoreRegisters *USB_CORE;
static UsbHostRegisters *USB_HOST;
static reg32 *USB_POWER;

static dword usbChannels = 0;
static bool rootPortEnabled = false;

bool dwhci_request_sync(UsbRequest *req, dword timeout);

