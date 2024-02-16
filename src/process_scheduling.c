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

// Comparison function for sorting by arrival time
int compare_by_arrival(const void *a, const void *b) {
    const ProcessControlBlock_t *pcb_a = *(const ProcessControlBlock_t **)a;
    const ProcessControlBlock_t *pcb_b = *(const ProcessControlBlock_t **)b;

    // Compare arrival times
    if (pcb_a->arrival < pcb_b->arrival) return -1;
    if (pcb_a->arrival > pcb_b->arrival) return 1;
    return 0;
}

bool first_come_first_serve(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    // Check for NULL parameters
    if (ready_queue == NULL || result == NULL) {
        return false;
    }

    // Sort ready_queue by arrival time
    if (!dyn_array_sort(ready_queue, compare_by_arrival)) {
        // Error while sorting
        return false;
    }

    // Store values for the schedule result stats
    unsigned long time = 0;
    size_t num_PCBs = 0;
    size_t total_wait_time = 0;

    // While ready queue is not empty keep going
    while (!dyn_array_empty(ready_queue)){
        // time when the PCB starts running = that PCBs wait time
        // NEEDS TO ACCOUNT FOR ARRIVAL TIME
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
    // Open the file for reading in binary mode
    FILE *file = fopen(input_file, "rb");
    if (!file) {
        return NULL; // Error opening file
    }

    // Create a dynamic array to hold the ProcessControlBlock_t elements
    dyn_array_t *pcb_array = dyn_array_create(sizeof(ProcessControlBlock_t), 10, NULL);
    if (!pcb_array) {
        fclose(file); // Clean up and exit if dynamic array creation failed
        return NULL;
    }

    // Read the burst time values from the file and populate the dynamic array
    uint32_t burst_time;
    while (fread(&burst_time, sizeof(uint32_t), 1, file) == 1) {
        ProcessControlBlock_t pcb = {0, 0, 0, 0}; // Initialize PCB to zero
        pcb.remaining_burst_time = burst_time; // Set the burst time
        
        // Insert the PCB into the dynamic array
        if (!dyn_array_push_back(pcb_array, &pcb)) {
            // Clean up in case of error
            dyn_array_destroy(pcb_array);
            fclose(file);
            return NULL;
        }
    }

    // Close the file
    fclose(file);

    // Return the populated dynamic array
    return pcb_array;
}

bool shortest_remaining_time_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    UNUSED(ready_queue);
    UNUSED(result);
    return false;
}
