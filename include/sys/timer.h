#pragma once

#include <common.h>

void timer_delay(dword ms);
dword timer_ticks();
qword clock_get_ticks();
void clock_init();
qword clock_physical_ticks();

void timer_delay_ms(dword ms);
void timer_delay_us(dword ms);

