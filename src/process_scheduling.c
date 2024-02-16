#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "dyn_array.h"
#include "processing_scheduling.h"


// You might find this handy.  I put it around unused parameters, but you should
// remove it before you submit. Just allows things to compile initially.
#define UNUSED(x) (void)(x)

// private function
void virtual_cpu(ProcessControlBlock_t *process_control_block) 
{
    // decrement the burst time of the pcb
    --process_control_block->remaining_burst_time;
}

bool first_come_first_serve(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    // Check for NULL parameters
    if (ready_queue == NULL || result == NULL) {
        return false;
    }

    // Is the ready_queue already sorted by arrival time?

    unsigned long time = 0;
    size_t num_PCBs = 0;
    size_t total_wait_time = 0;

    // While ready queue is not empty keep going
    while (!dyn_array_empty(ready_queue)){
        // time when the PCB starts running = that PCBs wait time
        total_wait_time += time;
        // While there is still burst time on the first PCB
        while (dyn_array_front(ready_queue) != NULL && dyn_array_front(ready_queue)->remaining_burst_time > 0) {
            // Run the PCB on the virtual cpu
            virtual_cpu(dyn_array_front(ready_queue));
            dyn_array_front(ready_queue)->started = true;
            ++time;
        }
        // Pop finished PCB
        if (!dyn_array_pop_front(ready_queue)) {
            // error with pop
            return false;
        }
        ++num_PCBs;
    }
    result->average_waiting_time = (float)total_wait_time / num_PCBs;
    result->average_turnaround_time = (float)time / num_PCBs;
    result->total_run_time = time;
    return true;
}

bool shortest_job_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    UNUSED(ready_queue);
    UNUSED(result);
    return false;   
}

bool priority(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    UNUSED(ready_queue);
    UNUSED(result);
    return false;   
}

bool round_robin(dyn_array_t *ready_queue, ScheduleResult_t *result, size_t quantum) 
{
    UNUSED(ready_queue);
    UNUSED(result);
    UNUSED(quantum);
    return false;
}

dyn_array_t *load_process_control_blocks(const char *input_file) 
{
    UNUSED(input_file);
    return NULL;
}

bool shortest_remaining_time_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    UNUSED(ready_queue);
    UNUSED(result);
    return false;
}
