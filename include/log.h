#pragma once

#include <common.h>

typedef enum {
    LTMUART,
    LTCONSOLE,
    LTFILE
} LogType;

void log_init(LogType t);

void log_putch(char c);
void log_print(char *fmt, ...);
void log_println(char *fmt, ...);
void log_num(dword n);
void log_hex(dword n, byte size);
void log_bin(dword n, byte size);
void log_dump(byte *b, dword size, byte columns);
