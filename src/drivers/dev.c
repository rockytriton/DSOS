#include <dev/dev.h>
#include <lib/dsstring.h>
#include <log.h>
#include "spi/spi.h"

#define MAX_DEVICES 20

static IODevice *devices[MAX_DEVICES];
static int numDevices = 0;

static int init_devices() {
    for (int i=0; i<numDevices; i++) {
        devices[i] = NULL;
    }

    return 0;
}

bool dev_register(IODevice *dev) {
    if (numDevices >= MAX_DEVICES) {
        return false;
    }

    bool found = false;

    for (int i=0; i<MAX_DEVICES; i++) {
        if (devices[i] == NULL) {
            devices[i] = dev;
            found = true;
            break;
        }
    }

    numDevices++;
    return found;
}

void dev_unregister(IODevice *dev) {

}

IODevice *dev_find(char *devName) {
    for (int i=0; i<numDevices; i++) {
        if (str_equal(devices[i]->name, devName)) {
            return devices[i];
        }
    }

    return NULL;
}

void timer_sys_init();
int dev_muart_init();
void dev_init_usb();
void spi_init_master();
void lcd_init();
void video_init();

void dev_load_devices() {
    init_devices();

    dev_muart_init();
    log_init(LTMUART);

    log_println(" ");
    log_println(" ");
    log_println(" ");

    timer_sys_init();
    clock_init();

    log_println("INIT VIDEO: ");
	video_init();

    //spi_init_master();
    

    log_println("INITIALZIE SPI");
    //spi_init_master();

    log_println("INIT VIDEO: ");
    //video_init();
    
    //lcd not yet ready...
    //lcd_init();

    //char *msg = "OHAI";
    //spi_send_command(0x53, 4, msg);

    //USB not yet ready...
    //dev_init_usb();
}
