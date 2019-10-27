#ifndef	_BOOT_H
#define	_BOOT_H

#ifdef __cplusplus
#include "common.h"
extern "C" {
#endif

extern void delay ( unsigned long);
extern void put32 ( unsigned long, unsigned int );
extern unsigned int get32 ( unsigned long );

#ifdef __cplusplus
}
void memcpy(byte *dest, byte *source, dword len);
#endif


#endif  /*_BOOT_H */
