#ifndef	_TIMER_H
#define	_TIMER_H

#include <common.h>

void timer_init ( void );
void handle_timer_irq ( void );
void handle_timer_irq2 ( void );
qword clock_get_ticks();

#endif  /*_TIMER_H */
