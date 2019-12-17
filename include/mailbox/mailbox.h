#pragma once

#include <common.h>

#define MBX_TAG_SET_POWER_STATE 0x00028001
#define MBX_TAG_GET_CLOCK_RATE 0x00030002

typedef struct {
    dword tagId;
    dword bufferSize;
    dword valueLength;
} MailboxTag PACKED;

typedef struct {
    dword commandSize;
    void *mailboxTag;
} MailboxCommand;

typedef struct {
    MailboxTag tagData;
        dword deviceId;
        dword state;
} MbxPowerData PACKED;

typedef struct {
    MailboxTag tagData;
    dword clockId;
    dword rate;
} MbxClockData PACKED;

typedef struct {
    MailboxTag tagData;
    byte transferData[32];
} MailboxTransfer PACKED;

typedef enum {
    CTEmmc = 1,
    CTUart = 2,
    CTArm = 3,
    CTCore = 4
} ClockType;

typedef enum {
    DPTSDCard = 0,
    DPTUsb = 3
} DevicePowerType;

dword mailbox_clock_rate(ClockType ct);

bool mailbox_power(DevicePowerType pt, bool on);

bool mailbox_process(dword tagId, MailboxTag *tag, dword tagSize);
