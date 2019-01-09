#pragma once

typedef enum {
    BlockDevUnknown,
    BlockDevRemovable,
    BlockDevPermanent
} BlockDeviceType;

typedef enum {
    OpRead,
    OpWrite,
    OpGetStatus
} BlockDevOp;

typedef struct {
    char name[11];
    unsigned char* buffer;
    unsigned int name_length;
    BlockDeviceType type;
    
    int(*init)(void);

    // For reading, arg = Sector to read, arg2 = Buffer to read to
    unsigned int(*operation)(BlockDevOp opCode, void* arg, void *arg2);
    void(*cleanup)(void);
} BlockDevice;

