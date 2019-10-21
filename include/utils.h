#ifndef	_BOOT_H
#define	_BOOT_H

#ifdef __cplusplus
extern "C" {
#endif

extern void delay ( unsigned long);
extern void put32 ( unsigned long, unsigned int );
extern unsigned int get32 ( unsigned long );

#ifdef __cplusplus
}
#endif

#endif  /*_BOOT_H */
