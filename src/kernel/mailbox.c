#include "mm.h"
#include "mailbox.h"

void dmb() {
    
}

void mailboxWrite(uint8_t channel, uint32_t data) {
    while(readMmio(MAIL_BASE + MAIL_STATUS) & MAIL_FULL) {

    }

    writeMmio(MAIL_BASE + MAIL_WRITE, (data & 0xfffffff0) | (uint32_t) (channel & 0xf));
}

uint32_t mailboxRead(uint8_t channel) {
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

