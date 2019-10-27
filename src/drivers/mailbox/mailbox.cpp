#include "./mailbox.h"
#include "mm.h"
#include "mailbox.h"
#include "logger.h"

namespace dsos {
namespace mailbox {

    const int VALUE_LENGTH_RESPONSE = (1 << 31);

    const int CODE_REQUEST = 0x00000000;
	const int CODE_RESPONSE_SUCCESS	= 0x80000000;
	const int CODE_RESPONSE_FAILURE	= 0x80000001;
    const int PROPTAG_END = 0;

    #define MAIL_BASE   (PBASE +   0xB880) // Mailbox Base Address

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


    struct PropertyBuffer {
        dword size;
        dword code;
        byte tags[0];
    };

    static Mailbox mailbox;

    static void mailboxWrite(uint8_t channel, uint32_t data) {
        while(readMmio(MAIL_BASE + MAIL_STATUS) & MAIL_FULL) {

        }

        writeMmio(MAIL_BASE + MAIL_WRITE, (data & 0xfffffff0) | (uint32_t) (channel & 0xf));
    }

    static uint32_t mailboxRead(uint8_t channel) {
        while(1) {
            while(readMmio(MAIL_BASE + MAIL_STATUS) & MAIL_EMPTY) {

            }

            uint32_t data = readMmio(MAIL_BASE + MAIL_READ);
            uint8_t readChannel = (uint8_t)(data & 0xf);

            if (readChannel == channel) {
                return data & 0xfffffff0;
            }
        }
    }

    Mailbox *Mailbox::inst() {
        return &mailbox;
    }

    static Logger &logger = Logger::inst();

    //static int pt[8192] __attribute__((aligned(16)));

    bool Mailbox::processTag(dword tagId, Tag &tag, dword tagSize) {
        tag.tagId = tagId;
        tag.bufferSize = tagSize - sizeof(Tag);
        tag.valueLength = 0;

        int bufferSize = tagSize + 8 + 4;

        //logger << "PROCESS TAG: " << tagSize << " - " << sizeof(Tag) << "\r\n";

        byte *buffer = new byte[bufferSize + 8];
        byte *source = (byte *)&tag;

        for (int i=0; i<tagSize; i++) {
            buffer[i + 8] = source[i];
        }

        PropertyBuffer *buff = (PropertyBuffer *)buffer;
        buff->size = bufferSize;
        buff->code = CODE_REQUEST;
        *(dword *)(buffer[8 + tagSize]) = PROPTAG_END;

        mailboxWrite(MAIL_TAGS, (dword)(uint64_t)buffer);
        int result = mailboxRead(MAIL_TAGS);

        for (int i=0; i<tagSize; i++) {
            source[i] = buffer[i + 8];
        }
    }
}
}

