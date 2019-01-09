#pragma once

#define MAX_TASKS 16

#define TASK_RUNNING 0

#define THREAD_SIZE 4096

struct CpuContext {
	unsigned long x19;
	unsigned long x20;
	unsigned long x21;
	unsigned long x22;
	unsigned long x23;
	unsigned long x24;
	unsigned long x25;
	unsigned long x26;
	unsigned long x27;
	unsigned long x28;
	unsigned long fp;
	unsigned long sp;
	unsigned long pc;
};

struct Task {
	struct CpuContext context;
	long state;	
	long counter;
	long priority;
	long preemptCount;
    int pId;
    struct Task *prev;
    struct Task *next;
};

extern struct Task *current;
extern struct Task *first;
extern struct Task *last;
extern int numTasks;

void schedule();
void preemptDisable();
void preemptEnable();
void terminateProcess();
