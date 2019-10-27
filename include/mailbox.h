#pragma once

#include "peripherals/base.h"

#ifdef __cplusplus 
extern "C" {
#endif

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

#define CLOCK_ID_EMMC		1
#define CLOCK_ID_UART		2
#define CLOCK_ID_ARM		3
#define CLOCK_ID_CORE		4

void mailboxWrite(uint8_t channel, uint32_t data);
uint32_t mailboxRead(uint8_t channel);

void addTag(int tag, int arg1, int arg2);


int getTag(int tag) ;
void *getTagData(int tag);

void initTags() ;

void processTags() ;


#ifdef __cplusplus 
}
#endif
