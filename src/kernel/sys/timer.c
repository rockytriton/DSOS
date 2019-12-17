#include <sys/timer.h>
#include <peripherals/base.h>
#include <sys/irq.h>

const dword CLOCKHZ = 1000000;

typedef struct {
    reg32 controlStatus;
    reg32 counterLow;
    reg32 counterHigh;
    reg32 compare[4];  
} TimerRegs;

TimerRegs *SYSTIMER;

static reg32 systemTicks = 0;
static reg32 interval = 0;
static reg32 curVal = 0;

void onSysTimer(SysIrq irq, void *userData) {
    //log_println("ON SYS TIMER: %d", systemTicks);
    //curVal = SYSTIMER->compare[3];
    curVal += (interval);
    SYSTIMER->compare[3] = curVal;
    SYSTIMER->controlStatus &= (1 << 3);

    systemTicks++;
}

void timer_sys_init() {
    SYSTIMER = (TimerRegs *)(PBASE + 0x00003000);
    irq_set_handler(SITimer3, onSysTimer, 0);
    interval = CLOCKHZ / 1000;
    curVal = SYSTIMER->compare[3] = (SYSTIMER->counterLow + interval);

    log_println("Initializing Timer: %8X", SYSTIMER);
    //timer_delay(1000);
    log_println("Timer Initialized");
}

void timer_delay(dword ms) {
    //timer_delay_ms(ms);
    
    dword start = systemTicks;
    reg32 goal = ms + start + 1;
    dword lastTick = start;

    while(systemTicks < goal) {
        //log_println("NOT YET");
    }
}

dword timer_ticks() {
    return systemTicks;
}

void timer_delay_ms(dword ms) {
    timer_delay_us(1000 * ms);
}

void timer_delay_us(dword us) {
    qword numTicks = us * (CLOCKHZ / 1000000) + 1;
    qword startTicks = clock_get_ticks();

    while(clock_get_ticks() - startTicks < numTicks);
}
