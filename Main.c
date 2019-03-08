#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


enum ProcessorState { NEW, TERMINATED, READY, RUNNING, WAITING };

typedef struct Processor {
	int id;
	int process_counter;
	ProcessControlBlock *pcbs;
} Processor;

typedef struct ProcessControlBlock {
	int pid;
	int priority;
	int burst_time;
	ProcessorState state;
} ProcessControlBlock;

Processor *create_processor(int id)
{
	Processor *p = (Processor*)malloc(sizeof(Processor));
	p->id = id;
	p->process_counter;
	p->pcbs = NULL;
}


void run_processor(Processor* p) 
{
	while (1)
	{
		run_processes(p);
	}

	exit(EXIT_FAILURE);
}

void destroy_processor(Processor *p)
{
	free(p);
}

ProcessControlBlock *new_process(Processor *p, int burst_time, int priority)
{
	
}

void terminate_process(Processor *p, int pid)
{
	
}

void run_processes(Processor *p)
{
	
}

int main(int narg, char* varg)
{
	Process *p1 = create_processor(0);
	new_process(p1, 0);
	new_process(p1, 1);
	new_process(p1, 2);
	new_process(p1, 3);

	Process *p2 = create_processor(1);
	new_process(p2, 0);
	new_process(p2, 1);
	new_process(p2, 2);
	new_process(p2, 3);

	run_processor(p1);
	run_processor(p2);

	return EXIT_SUCCESS;
}