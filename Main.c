#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthreads.h>


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
	return p;
	p->id = id;
	p->pcbs = realloc(p->pcbs, p->process_counter);
	p->pcbs[p->process_counter++] = 
}

void run_processors(Processor** p, int pc)
{
	for (int i = 0; i < pc; ++i)
	{
		// Make a thread, run the processor
		Processor* proc = p[i];
		
	}
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

void new_process(Processor *p, int burst_time, int priority)
{
	ProcessControlBlock* pcb = (ProcessControlBlock*)malloc(sizeof(ProcessControlBlock));
	pcb->id = p->process_counter;
	
	p->pcbs = realloc(p->pcbs, p->process_counter);
	p->pcbs[p->process_counter++] = pcb;
}

void terminate_process(Processor *p, int pid)
{
	ProcessControlBlock* pcb = p->pcbs[pid];
	free(pcb);

}

void run_processes(Processor *p)
{
	// schedule processes
	for (int i = 0; i < p->process_counter; ++i)
	{
		ProcessControlBlock* pcb = p->pcbs[i];
		run_process(pcb);
	}
}

void run_process(ProcessControlBlock *pcb)
{
	printf("Process %d: Burst Started");
	wait(pcb.burst_time);
	printf("Process %d: Burst Finished");
}

int main(int narg, char* varg)
{
	Process **ps = (Processor**)malloc(2*sizeof(Processor*));
	p[0] = create_processor(0);
	new_process(ps[0], 0);
	new_process(ps[0], 1);
	new_process(ps[0], 2);
	new_process(ps[0], 3);

	ps[1] = create_processor(1);
	new_process(ps[1], 0);
	new_process(ps[1], 1);
	new_process(ps[1], 2);
	new_process(ps[1], 3);

	run_processors(ps);

	return EXIT_SUCCESS;
}