// Gabriel Anderson
// CS470
// Lab 5
//
// Building: gcc Main.c -lpthread

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

/* Global Variables */

int uniq = 0;

/* Struct Declarations */

typedef enum ProcessorState ProcessorState;
typedef enum SchedulerType SchedulerType;
typedef struct ProcessControlBlock ProcessControlBlock;
typedef struct Processor Processor;


/* Struct Definitions */
enum ProcessorState { NEW, TERMINATED, READY, RUNNING, WAITING };
enum SchedulerType { SJF, PRIORITY };

typedef struct Processor {
	int id;
	int working;
	int process_counter;
	int max_processes;
	SchedulerType scheduler;
	Processor* neighbor;
	pthread_mutex_t lock;

	ProcessControlBlock* new_processes;
	int new_processes_size;

	ProcessControlBlock* ready_processes;
	int ready_processes_size;

	ProcessControlBlock* running_processes;
	int running_processes_size;

	ProcessControlBlock* waiting_processes;
	int waiting_processes_size;

	ProcessControlBlock* terminated_processes;
	int terminated_processes_size;

} Processor;

typedef struct ProcessControlBlock {
	int pid;
	int priority;
	int burst_time;
	int burst_count;
	int burst_cap;
	ProcessorState state;
} ProcessControlBlock;


/* Function Declarations */
Processor *create_processor(int id, int max_processes, SchedulerType scheduler);
void run_processors(Processor** p, int pc);
void *run_processor_worker(void* proc_vptr);
void run_processor(Processor* p);
void destroy_processor(Processor *p);
void new_random_process(Processor *p);
void new_process(Processor *p, int burst_time, int bursts, int priority);
void terminate_process(Processor *p, int pid);
void run_processes(Processor *p);
void run_process(ProcessControlBlock *pcb);
void* run_aging(void* proc_vptr);
void* run_status_mimick(void* proc_vptr);
int randomInt(int max);
void sortSJF(ProcessControlBlock *pcb, int size);
void sortPriority(ProcessControlBlock *pcb, int size);
void shuffle(ProcessControlBlock *pcb, int size);
void load_balance(Processor* p1, Processor* p2);


/* Function Definitions */
Processor *create_processor(int id, int max_processes, SchedulerType scheduler)
{
	printf("Creating processor %d...\n", id);

	Processor *p = (Processor*)malloc(sizeof(Processor));
	p->id = id;
	p->working = 0;
	p->max_processes = max_processes;
	p->process_counter = 0;
	p->scheduler = scheduler;

	p->new_processes = (ProcessControlBlock*)malloc(max_processes*sizeof(ProcessControlBlock));
	p->new_processes_size = 0;
	
	p->ready_processes = (ProcessControlBlock*)malloc(max_processes*sizeof(ProcessControlBlock));
	p->ready_processes_size = 0;

	p->running_processes = (ProcessControlBlock*)malloc(max_processes*sizeof(ProcessControlBlock));
	p->running_processes_size = 0;

	p->waiting_processes = (ProcessControlBlock*)malloc(max_processes*sizeof(ProcessControlBlock));
	p->waiting_processes_size = 0;

	p->terminated_processes = (ProcessControlBlock*)malloc(max_processes*sizeof(ProcessControlBlock));
	p->terminated_processes_size = 0;

	if (pthread_mutex_init(&p->lock, NULL) != 0) 
    { 
        fprintf(stderr, "Mutex init has failed\n"); 
        exit(EXIT_FAILURE); 
    }

	printf("Processor %d created successfully!\n", id);
	return p; 
}

void run_processors(Processor** p, int pc)
{
	printf("Running processors...\n");

	pthread_t *proc_threads = (pthread_t*)malloc(pc*sizeof(pthread_t));
	pthread_t *aging_threads = (pthread_t*)malloc(pc*sizeof(pthread_t));
	pthread_t *status_mimick_threads = (pthread_t*)malloc(pc*sizeof(pthread_t));

	for (int i = 0; i < pc; ++i)
	{
		Processor* proc = p[i];
		proc->working = 1;

		printf("Processor %d running...\n", proc->id);

		// Run the processor in a thread
		if(pthread_create(&proc_threads[i], NULL, run_processor_worker, proc))
		{
			fprintf(stderr, "Error creating thread\n");
			exit(EXIT_FAILURE);
		}

		// Age this processors processes
		if(pthread_create(&aging_threads[i], NULL, run_aging, proc))
		{
			fprintf(stderr, "Error creating thread\n");
			exit(EXIT_FAILURE);
		}

		// Mimick status changes on this process
		if(pthread_create(&status_mimick_threads[i], NULL, run_status_mimick, proc))
		{
			fprintf(stderr, "Error creating thread\n");
			exit(EXIT_FAILURE);
		}
	}

	for (int i = 0; i < pc; ++i)
	{
		pthread_join(proc_threads[i], NULL);
		pthread_join(aging_threads[i], NULL);
		pthread_join(status_mimick_threads[i], NULL);
		printf("Processor %d completed.\n", p[i]->id);
	}

	free(proc_threads);
	free(aging_threads);
	free(status_mimick_threads);
}

void *run_processor_worker(void* proc_vptr)
{
	Processor* proc = (Processor*)proc_vptr;
	run_processor(proc);
	return NULL;
}

void run_processor(Processor* p) 
{
	while (p->new_processes_size > 0 || p->ready_processes_size > 0 || p->waiting_processes_size > 0 || p->running_processes_size > 0)
	{
		run_processes(p);
		printf("Processor %d Status: %d Ready, %d Waiting\n", p->id, p->ready_processes_size, p->waiting_processes_size);
	}

	p->working = 0;
}

void destroy_processor(Processor *p)
{
	pthread_mutex_destroy(&p->lock);
	free(p);
}

void new_random_process(Processor *p)
{
	int burst_time = randomInt(6);
	int bursts = randomInt(5)+1;
	int priority = randomInt(128);
	new_process(p, burst_time, bursts, priority);
}

void new_process(Processor *p, int burst_time, int bursts, int priority)
{
	int pid = uniq++;
	printf("Processor %d: Spawning process %d...\n", p->id, pid);

	if (p->process_counter > p->max_processes)
	{
		fprintf(stderr, "Processor %d: Cannot create a new process. Capacity reached.\n", p->id);
		return;
	}

	ProcessControlBlock pcb;
	pcb.pid = pid;
	pcb.priority = priority;
	pcb.burst_count = 0;
	pcb.burst_cap = bursts;
	pcb.burst_time = burst_time;
	pcb.state = NEW;

	p->new_processes[p->new_processes_size++] = pcb;

	printf("Processor %d: Process %d spawned successfully\n", p->id, pid);
}

void terminate_process(Processor *p, int pid)
{
	free(p->new_processes);
	free(p->ready_processes);
	free(p->running_processes);
	free(p->waiting_processes);
	free(p->terminated_processes);
}

void run_processes(Processor *p)
{
	// Move new processes to the ready queue
	pthread_mutex_lock(&p->lock); 
	for (int i = p->new_processes_size-1; i >= 0; --i)
	{
		printf("(%d) Process %d: New -> Ready\n", p->id, p->new_processes[i].pid);
		p->ready_processes[p->ready_processes_size++] = p->new_processes[i];
	}
	p->new_processes_size = 0;
	pthread_mutex_unlock(&p->lock); 

	// Move processes on the ready queue into the running queue
	pthread_mutex_lock(&p->lock);
	for (int i = p->ready_processes_size-1; i >= 0; --i)
	{
		printf("(%d) Process %d: Ready -> Running\n", p->id, p->ready_processes[i].pid);
		p->running_processes[p->running_processes_size++] = p->ready_processes[i];
	}
	p->ready_processes_size = 0;
	pthread_mutex_unlock(&p->lock);

	// Sort the processes in the running queue based on the cpu scheduler
	if (p->scheduler == SJF)
		sortSJF(p->running_processes, p->running_processes_size);
	else // Only other scheduler is priority-based
		sortPriority(p->running_processes, p->running_processes_size);

	for (int i = p->running_processes_size-1; i >= 0; --i)
	{
		ProcessControlBlock pcb = p->running_processes[i];

		pthread_mutex_lock(&p->lock); 
			run_process(&pcb);
		
		// Add finished processes to either waiting or running
		if (pcb.burst_count >= pcb.burst_cap)
		{
			// With no more burst left, the process is complete.
			pcb.state = TERMINATED;
			p->terminated_processes[p->terminated_processes_size++] = pcb;
			printf("(%d) Process %d: Running -> Terminated\n", p->id, pcb.pid);
		}
		else if(randomInt(2) == 0)
		{
			// Put pcb in waiting
			pcb.state = WAITING;
			p->waiting_processes[p->waiting_processes_size++] = pcb;
			printf("(%d) Process %d: Running -> Waiting\n", p->id, pcb.pid);
		}
		else
		{
			// Put pcb in ready
			pcb.state = READY;
			p->ready_processes[p->ready_processes_size++] = pcb;
			printf("(%d) Process %d: Running -> Ready\n", p->id, pcb.pid);
		}

		pthread_mutex_unlock(&p->lock);
	}
	p->running_processes_size = 0;

	// Try to load balance??
	pthread_mutex_lock(&p->neighbor->lock);
	pthread_mutex_lock(&p->lock);
	load_balance(p, p->neighbor);
	pthread_mutex_unlock(&p->lock);
	pthread_mutex_unlock(&p->neighbor->lock);
	
}

void run_process(ProcessControlBlock *pcb)
{
	pcb->burst_count++;
	printf("Process %d: %ds Burst %d/%d started...\n", pcb->pid, pcb->burst_time, pcb->burst_count, pcb->burst_cap);
	sleep(pcb->burst_time);
	printf("Process %d: %ds Burst %d/%d Finished\n", pcb->pid, pcb->burst_time, pcb->burst_count, pcb->burst_cap);
}


int randomInt(int max)
{
	return rand() % max;
}

void* run_aging(void* proc_vptr)
{
	Processor* p = (Processor*)proc_vptr;

	while (p->working != 0)
	{
		pthread_mutex_lock(&p->lock);

		printf("Aging processes...\n");

		// Iterate through active processes, increasing their priority by 10.
		for (int i = 0; i < p->ready_processes_size; ++i) {
			p->ready_processes[i].priority -= 10;
			if (p->ready_processes[i].priority < 0)
				p->ready_processes[i].priority = 0;
		}

		for (int i = 0; i < p->running_processes_size; ++i) {
			p->running_processes[i].priority -= 10;
			if (p->running_processes[i].priority < 0)
				p->running_processes[i].priority = 0;
		}

		for (int i = 0; i < p->waiting_processes_size; ++i) {
			p->waiting_processes[i].priority -= 10;
			if (p->waiting_processes[i].priority < 0)
				p->waiting_processes[i].priority = 0;
		}

		pthread_mutex_unlock(&p->lock);

		sleep(2); // Invoked every two seconds
	}
}


void* run_status_mimick(void* proc_vptr)
{
	Processor* p = (Processor*)proc_vptr;

	while (p->working != 0)
	{
		if (p->waiting_processes_size == 0) continue;

		pthread_mutex_lock(&p->lock);

		printf("Mimicking status updates...\n");

		// Shuffle waiting queue
		shuffle(p->waiting_processes, p->waiting_processes_size);

		// Move random number of elements from waiting to running
		int elems = randomInt(p->waiting_processes_size);
		for (int i = 0; i <= elems; i++)
		{
			p->ready_processes[p->ready_processes_size++] = p->waiting_processes[--p->waiting_processes_size];
		}

		pthread_mutex_unlock(&p->lock);

		sleep(5); // Invoked every 5 seconds
	}
	
}


int sjf_comparator(const void *p, const void *q)  
{ 
    int l = ((ProcessControlBlock*)p)->burst_time; 
    int r = ((ProcessControlBlock*)q)->burst_time;  
    return (l - r); 
} 


void sortSJF(ProcessControlBlock *pcb, int size)
{
	qsort((void*)pcb, size, sizeof(ProcessControlBlock), sjf_comparator);
}

int priority_comparator(const void *p, const void *q)  
{ 
    int l = ((ProcessControlBlock*)p)->priority; 
    int r = ((ProcessControlBlock*)q)->priority;  
    return (l - r); 
} 

void sortPriority(ProcessControlBlock *pcb, int size)
{
	qsort((void*)pcb, size, sizeof(ProcessControlBlock), priority_comparator);
}

void shuffle(ProcessControlBlock *pcb, int size)
{
	if (size > 1)
	{
		for (int i = 0; i < size; ++i)
		{
			int j = randomInt(size);
			ProcessControlBlock tmp = pcb[i];
			pcb[i] = pcb[j];
			pcb[j] = tmp;
		}
	}
}

void load_balance(Processor* p1, Processor* p2)
{
	if(p1->ready_processes_size == 0 && p2->ready_processes_size > 1)
	{
		printf("Load balancing\n");
		int elems = p2->ready_processes_size / 2;
		for (int i = 0; i <= elems; ++i)
		{
			p1->ready_processes[p1->ready_processes_size++] = p2->ready_processes[--p2->ready_processes_size];
		}
	}
	else if (p1->ready_processes_size > 1 && p2->ready_processes_size == 0)
	{
		printf("Load balancing\n");
		int elems = p1->ready_processes_size / 2;
		for (int i = 0; i <= elems; ++i)
		{
			p2->ready_processes[p2->ready_processes_size++] = p1->waiting_processes[--p1->waiting_processes_size];
		}
	}
}


int main(int nargs, char** vargs)
{
	srand(time(0));

	if (nargs != 3)
	{
		fprintf(stderr, "Invalid number of arguments.\n");
		printf("Usage: %s <n> <m>\n", vargs[0]);
		exit(EXIT_FAILURE);
	}

	int n = atoi(vargs[1]);
	int m = atoi(vargs[2]);

	Processor **ps = (Processor**)malloc(2*sizeof(Processor*));
	ps[0] = create_processor(0, n+m, SJF);
	for (int i = 0; i < n; ++i)
		new_random_process(ps[0]);

	ps[1] = create_processor(1, n+m, PRIORITY);
	for (int i = 0; i < m; ++i)
		new_random_process(ps[1]);

	ps[0]->neighbor = ps[1];
	ps[1]->neighbor = ps[0];

	run_processors(ps, 2);

	return EXIT_SUCCESS;
}