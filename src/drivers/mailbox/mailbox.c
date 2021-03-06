#include <mailbox/mailbox.h>
#include <dev/dev.h>
#include <peripherals/base.h>
#include <log.h>
#include "mm.h"

const int VALUE_LENGTH_RESPONSE = (1 << 31);

const int CODE_REQUEST = 0x00000000;
const int CODE_RESPONSE_SUCCESS	= 0x80000000;
const int CODE_RESPONSE_FAILURE	= 0x80000001;
const int PROPTAG_END = 0;

#define MAIL_READ      0x00 // Mailbox Read Register
#define MAIL_CONFIG    0x1C // Mailbox Config Register
#define MAIL_STATUS    0x18 // Mailbox Status Register
#define MAIL_WRITE     0x20 // Mailbox Write Register

#define MAIL_EMPTY  0x40000000 // Mailbox Status Register: Mailbox Empty (There is nothing to read from the Mailbox)
#define MAIL_FULL   0x80000000 // Mailbox Status Register: Mailbox Full  (There is no space to write into the Mailbox)

#define MAIL_POWER    0x0 // Mailbox Channel 0: Power Management Interface
#define MAIL_FB       0x1 // Mailbox Channel 1: Frame Buffer
#define MAIL_VUART    0x2 // Mailbox Channel 2: Virtual UART
#define MAIL_VCHIQ    0x3 // Mailbox Channel 3: VCHIQ Interface
#define MAIL_LEDS     0x4 // Mailbox Channel 4: LEDs Interface
#define MAIL_BUTTONS  0x5 // Mailbox Channel 5: Buttons Interface
#define MAIL_TOUCH    0x6 // Mailbox Channel 6: Touchscreen Interface
#define MAIL_COUNT    0x7 // Mailbox Channel 7: Counter
#define MAIL_TAGS     0x8 // Mailbox Channel 8: Tags (ARM to VC)

#define POWER_STATE_OFF		(0 << 0)
#define POWER_STATE_ON		(1 << 0)
#define POWER_STATE_WAIT	(1 << 1)
#define POWER_STATE_NO_DEVICE	(1 << 1)	// in response

#define PROPTAG_SET_POWER_STATE		0x00028001
#define PROPTAG_GET_CLOCK_RATE		0x00030002

typedef struct {
    reg32 read;
    reg32 res[5];
    reg32 status;
    reg32 config;
    reg32 write;
} MBRegisters;

inline MBRegisters *MBX() {
    return (MBRegisters *)(PBASE + 0xB880);
}

typedef struct {
    dword size;
    dword code;
    byte tags[0];
} PropertyBuffer;

static dword propertyData[8192] __attribute__((aligned(16)));

static int mailbox_write(byte channel, dword data) {
    while(MBX()->status & MAIL_FULL);

    MBX()->write = (data & 0xFFFFFFF0) | (channel & 0xF);
}

static dword mailbox_read(byte channel) {
    while(true) {
        while(MBX()->status & MAIL_EMPTY);

        dword data = MBX()->read;
        log_println("MBX READ: %X", data);

        byte readChannel = (byte)(data & 0xF);

        assert(readChannel == channel, "ERR");

        if (readChannel == channel) {
            return data & 0xFFFFFFF0;
        }
    }
}

void mailbox_old_school_req(dword *p, dword size) {
    memcpy(propertyData, p, size);

    log_println("Writing %X", propertyData);

    asm volatile ("dsb sy" ::: "memory");

    mailbox_write(MAIL_TAGS, (dword)(uint64_t)propertyData);

    log_println("waiting");

    int result = mailbox_read(MAIL_TAGS);

    asm volatile ("dmb sy" ::: "memory");

    log_println("Read: %X", result);

    memcpy(p, propertyData, size);
}

bool mailbox_process(dword tagId, MailboxTag *tag, dword tagSize) {
    log_println("mailbox_process: %X", tag);
    /*
    //tag->tagId = tagId;
    log_println("TadID: %X", tag->tagId);
    log_println("TadID: %X", tag->tagId);
    //tag->valueLength = 0;
    log_println("SOMETHING");
    //tag->bufferSize = tagSize - sizeof(MailboxTag);
    log_println("TadID: %X", tag->bufferSize);
*/

    int bufferSize = tagSize + 8 + 4;

    byte *buffer = (byte *)propertyData;
    byte *source = (byte *)tag;

    for (int i=0; i<tagSize; i++) {
        buffer[i + 8] = source[i];
    }

    PropertyBuffer *buff = (PropertyBuffer *)buffer;
    buff->size = bufferSize;
    buff->code = CODE_REQUEST;
    dword *pdwBuf = (dword *)buffer;
    pdwBuf[(tagSize + 12) / 4 - 1] = PROPTAG_END;
    //*(dword *)(buffer[8 + tagSize]) = PROPTAG_END;

    asm volatile ("dsb sy" ::: "memory");

    mailbox_write(MAIL_TAGS, (dword)(uint64_t)buffer);

    log_println("waiting");

    int result = mailbox_read(MAIL_TAGS);

    asm volatile ("dmb sy" ::: "memory");

    log_println("Read: %d", tagSize);

    for (int i=0; i<tagSize; i++) {
        source[i] = buffer[i + 8];
    }

    return true;
}

static bool mailbox_command(void *commandData) {
    MailboxCommand *cmd = (MailboxCommand *)commandData;
    MailboxTag *tag = (MailboxTag *)cmd->mailboxTag;

    return mailbox_process(tag->tagId, cmd, cmd->commandSize);
}

dword mailbox_clock_rate(ClockType ct) {
    MbxClockData cd __attribute__((aligned(16)));
    cd.tagData.tagId = PROPTAG_GET_CLOCK_RATE;
    cd.tagData.valueLength = 0;
    cd.tagData.bufferSize = sizeof(MbxClockData) - sizeof(MailboxTag);
    cd.clockId = ct;

    log_println("SIZE OF IT: %d - %X", sizeof(cd), &cd);

    if (!mailbox_process(PROPTAG_GET_CLOCK_RATE, (MailboxTag *)&cd, sizeof(cd))) {
        log_println("FAILED TO PROC MBX");
    }

    return cd.rate;
}

void mailbox_power_check(dword type) {
    MbxPowerData pd __attribute__((aligned(16)));
    pd.tagData.tagId = 0x00020001;
    pd.tagData.valueLength = 8;
    pd.tagData.bufferSize = sizeof(MbxPowerData) - sizeof(MailboxTag);
    pd.deviceId = type;

    mailbox_process(0x00020001, (MailboxTag *)&pd, sizeof(pd));

    log_println("GOT POWER STATE: %d - %d", pd.state, pd.tagData.valueLength);
}

bool mailbox_power(DevicePowerType pt, bool on) {
    MbxPowerData pd __attribute__((aligned(16)));
    pd.tagData.tagId = PROPTAG_SET_POWER_STATE;
    pd.tagData.valueLength = 0;
    pd.tagData.bufferSize = sizeof(MbxPowerData) - sizeof(MailboxTag);
    pd.deviceId = pt;

    if (on) {
        pd.state = POWER_STATE_ON | POWER_STATE_WAIT;
    } else {
        pd.state = POWER_STATE_OFF | POWER_STATE_WAIT;
    }

    mailbox_process(PROPTAG_SET_POWER_STATE, (MailboxTag *)&pd, sizeof(pd));

    return !(pd.state & POWER_STATE_NO_DEVICE);
}


static IODevice muart = {
    .command = mailbox_command,
    .name = "mailbox"
};

