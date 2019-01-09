#pragma once

typedef enum{
    UnknownFsType = 0x00,
    Fat12 = 0x01,
    XenixRoot = 0x02,
    XenixUser = 0x03,
    FAT16LessThan32Mb = 0x04,
    ExtendedMsDos = 0x05,
    FAT16Over32Mb = 0x06,
    Ntfs = 0x07, // Can also be IFS or exFAT
    Fat16Ext = 0x06,
    FAT32WithCHS = 0x0B, // Partitions up to 2047MB
    FAT32XWithLba = 0x0C, // Lba extension
    Fat16WithLba = 0xE,
    Fat32WithLba3 = 0xF,
    Linux = 0x93,
    LinuxSwap = 0x42,
    LinuxNative = 0x43,
    LinuxSwap2 = 0x82,
    LinuxNative2 = 0x83,
    LinuxExtended = 0x85,
    OsxBoot = 0xAB
} fs_type;

