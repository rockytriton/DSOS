#include "scheduler.h"

static struct Task initTask = { {0,0,0,0,0,0,0,0,0,0,0,0,0},0,0,1,0,0,0,0};

struct Task *current = &initTask;
struct Task *last = &initTask;
struct Task *first = &initTask;

int numTasks = 1;

void context_switch(struct Task *p, struct Task *n);

void enable_irq();
void disable_irq();

void preemptDisable() {
    current->preemptCount++;
}

void preemptEnable() {
    current->preemptCount--;
}

void doSchedule() {
    preemptEnable();
    struct Task *next;
    int counter;

    while(1) {
        counter = -1;
        next = 0;

        for (struct Task *p = first; p != 0; p = p->next) {
            if (p->state == TASK_RUNNING && p->counter > counter) {
                next = p;
                counter = next->counter;
            }
        }

        if (counter) {
            break;
        }

        for (struct Task *p = first; p != 0; p = p->next) {
            if (p->state == TASK_RUNNING) {
                p->counter = (p->counter >> 1) + p->priority;
            }
        }
    }

    if (next != current) {
        struct Task *prev = current;
        current = next;
        context_switch(prev, current);
    }

    preemptEnable();
}

void schedule() {
    current->counter = 0;
    doSchedule();
}

void schedulerTick() {
    current->counter--;

    if (current->counter > 0 || current->preemptCount > 0) {
        return;
    }

    current->counter = 0;
    enable_irq();
    doSchedule();
    disable_irq();
}

void terminateProcess() {
    preemptDisable();
    //printf("Terminating Process: %d (%x, %x)\r\n", current->pId, current->prev, current->next);

    if (current->prev != 0) {
        current->prev->next = current->next;
    }

    if (current->next != 0) {
        current->next->prev = current->prev;
    }

    current->state = 99;

    freePage(current);

    preemptEnable();

    //schedule();

    //while(1) {

    //}
}
