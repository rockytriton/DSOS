#pragma once

#include <common.h>

typedef qword (*DEV_SEEK)(qword n);
typedef int (*DEV_READ)(byte *bytes, int size);
typedef int (*DEV_WRITE)(byte *bytes, int size);
typedef bool (*DEV_COMMAND)(void *commandData);
typedef void (*DEV_INIT)();
typedef void (*DEV_DEINIT)();

typedef struct {
    DEV_SEEK seek;
    DEV_READ read;
    DEV_WRITE write;
    DEV_COMMAND command;
    DEV_INIT init;
    DEV_DEINIT deinit;
    char name[64];
} IODevice;

bool dev_register(IODevice *dev);
void dev_unregister(IODevice *dev);
IODevice *dev_find(char *devName);

