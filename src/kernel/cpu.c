#include <log.h>
#include "mm.h"

int getTemp() {
	/*
	uint32_t *mb_addr = (uint32_t *)allocPage();
	volatile uint32_t *mailbuffer = (uint32_t *)mb_addr;
	mailbuffer[0] = 8 * 4;
	mailbuffer[1] = 0;
	mailbuffer[2] = 0x00030006;
	mailbuffer[3] = 0x8;
	mailbuffer[4] = 0x8;
	mailbuffer[5] = 0;
	mailbuffer[6] = 0x0;
	mailbuffer[7] = 0x0;

	log_print("Checking Temp: \r\n");
	// send the message
	mailboxWrite(MAIL_TAGS, (uint32_t)(long)mb_addr);

	// read the response
	mailboxRead(MAIL_TAGS);

	log_print("   TEMP: %d\r\n", mailbuffer[6]);
	log_print("\r\n");
	*/

	return 0;
}
