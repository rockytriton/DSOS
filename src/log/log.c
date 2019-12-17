#include <log.h>
#include <lib/dsstring.h>
#include <dev/dev.h>
#include <stdarg.h>
#include "printf.h"
#include <peripherals/base.h>

static IODevice *ioDevice = NULL;
 int CPUVER = 0;

int getPN();

void putc (void* p, char c)
{
	ioDevice->write(&c, 1);
}

void log_init(LogType t) {
    if (t == LTMUART) {
        ioDevice = dev_find("miniuart");
    }
    CPUVER = getPN();
    log_println("MAILBOX STATE: %X - %X", PBASE, CPUVER);
}

void log_putch(char c) {
	ioDevice->write(&c, 1);
}

void log_println(char *fmt, ...) {
    va_list va;

    va_start(va,fmt);
    tfp_format(NULL, putc, fmt, va);
    va_end(va);

    char *p = "\r\n";
    ioDevice->write(p, 2);
}

void log_print(char *fmt, ...) {
    va_list va;

    va_start(va,fmt);
    tfp_format(NULL, putc, fmt, va);
    va_end(va);
}

void log_num(dword n) {
    log_print("%d", n);
}

void log_hex(dword n, byte size) {
    char buff[32];
    tfp_sprintf(buff, "%%%dX", size);

    log_print(buff, n);
}

void log_bin(dword n, byte size) {

}

void * memset (void *dest, int val, dword len)
{
  unsigned char *ptr = (byte *)dest;
  while (len-- > 0)
    *ptr++ = val;
  return dest;
}


void log_dump(byte *b, dword size, byte columns) {
    char ascii[256];
    memset(ascii, 0, 256);
    int a = 0;

    for (int i=0; i<size; i++, a++) {
        bool newRow = (i % columns) == 0;

        if (newRow) {
            log_print("    ");
            log_print(ascii);
            log_println(" ");
            memset(ascii, 0, 256);
            a = 0;
        }

        log_hex(((byte *)b)[i], 2);
        char c = (char)((byte *)b)[i];
        if (c >= 0x20)
            ascii[a] = c;
        else 
            ascii[a] = '.';

        log_putch(' ');
    }

    log_print("    ");
    log_print(ascii);
    log_println(" ");
}


