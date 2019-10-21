
#include "../drivers/miniuart/miniuart.h"
#include "printf.h"
#include "logger.h"
#include "mm.h"

extern "C" void kernel_main2(void);


extern "C" void uart_init();
extern "C" void uart_send(char c);
extern "C" void putc ( void* p, char c);

using namespace dsos;

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

    printf("TEST UNO\r\n");
    printf("String Size: %d\r\n", sizeof(String));

    printf("PAGE SIZE: %d\r\n", PAGE_SIZE);
    printf("PAGES    : %d\r\n", PAGING_PAGES);
    printf("PAGE MEM : %d\r\n", PAGING_MEMORY);

    printf("Last alloc Size: %d\r\n", (dword)PAGE_SIZE);
    printf("String Size: %d\r\n", stringLen("String"));

    char a[32];
    copyString(a, "TEST STRING");
    printf("String Size: %d\r\n", stringLen(a));

    logger.println(a);

    String s("TEST ONE");

    printf("Last alloc Size: %d\r\n", lastAllocSize);

    logger.println(s.c_str());

    String s2;
    toString(8675309, s2);

    logger.println(s2.c_str());

    logger << "TEST TWO\r\n";
    logger << "TEST " << s2.c_str() << "\r\n";

    logger << "TEST DOS" << "\r\n" << "TEST TRES\r\n";
    logger << "TEST " << 1 << "\r\n";
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

    while(1) {

    }
}
