
#include "../drivers/miniuart/miniuart.h"
#include "../drivers/gpio/gpio.h"
#include "printf.h"
#include "logger.h"
#include "mm.h"
#include "timer.h"

#include "utils.h"
#include "../drivers/emmc/emmc.h"

extern "C" void kernel_main2(void);


extern "C" void uart_init();
extern "C" void uart_send(char c);
extern "C" void putc ( void* p, char c);

using namespace dsos;
using namespace dsos::gpio;

extern int lastAllocSize;

Logger &logger = Logger::inst();

void cpp_kernel() {
    init_printf(0, putc);


    uart_send('\r');
    uart_send('\n');

    uart_send('H');
    uart_send('i');

    dsos::drivers::MiniUart::inst()->send("!!!");
    uart_send('\r');
    uart_send('\n');

    printf("TEST WITH GPIO DRIVER\r\n");
    
    logger << "TEST " << 8675309 << "\r\n";
    
    dsos::drivers::MiniUart::inst()->send("Initialized 2 C++ MiniUart\r\n");

    kernel_main2();

    dsos::drivers::MiniUart::inst()->send("Initialized 2 C++ Kernel\r\n");

    char *p1 = new char[300];
    char *p2 = new char[300];
    char *p3 = new char[300];

    delete[] p2;

    char *p4 = new char[200];
    char *p5 = new char[300];
    char *p6 = new char[200];

    logger.print("P1: ");
    logger.printHex((dword)(uint64_t)p1);
    logger.println(" ");
    logger.print("P2: ");
    logger.printHex((dword)(uint64_t)p2);
    logger.println(" ");
    logger.print("P3: ");
    logger.printHex((dword)(uint64_t)p3);
    logger.println(" ");

    logger.print("P4: ");
    logger.printHex((dword)(uint64_t)p4);
    logger.println(" ");

    logger.print("P5: ");
    logger.printHex((dword)(uint64_t)p5);
    logger.println(" ");

    logger.print("P6: ");
    logger.printHex((dword)(uint64_t)p6);
    logger.println(" ");

    bool on18 = false;
    timerInit();

    dsos::emmc::Emmc emmc;
    emmc.cardInit();


    while(1) {
            delayMs(1000);

            if (on18) {
                GPIO()->outputSet.values[0] |= (1 << 18);
                //logger.println("ON TIMER SET");
            } else {
                GPIO()->outputClear.values[0] |= (1 << 18);
                //logger.println("ON TIMER CLEAR");
            }
            
            on18 = !on18;
    }
}
