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
    const ProcessControlBlock_t *pcb_a = (const ProcessControlBlock_t *)a;
    const ProcessControlBlock_t *pcb_b = (const ProcessControlBlock_t *)b;

    // Compare arrival times
    if (pcb_a->arrival < pcb_b->arrival) return -1;
    if (pcb_a->arrival > pcb_b->arrival) return 1;
    return 0;
}

// Comparison function for sorting by burst time
int compare_by_burst_time(const void *a, const void *b) {
    const ProcessControlBlock_t *pcb_a = (const ProcessControlBlock_t *)a;
    const ProcessControlBlock_t *pcb_b = (const ProcessControlBlock_t *)b;

    // Compare arrival times
    if (pcb_a->remaining_burst_time < pcb_b->remaining_burst_time) return -1;
    if (pcb_a->remaining_burst_time > pcb_b->remaining_burst_time) return 1;
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
        // Get the next pcb
        ProcessControlBlock_t *current_pcb = dyn_array_front(ready_queue);
        if (current_pcb == NULL) {
            //error with dyn_array_front
            return false;
        }
        // time when the PCB starts running minus arrival time = that PCBs wait time
        total_wait_time += time - current_pcb->arrival;
        // While there is still burst time on the first PCB run the PCB on the virtual cpu
        while (current_pcb->remaining_burst_time > 0) {
            virtual_cpu(current_pcb);
            current_pcb->started = true;
            ++time;
        }
        // Pop finished PCB
        if (!dyn_array_pop_front(ready_queue)) {
            // error with pop
            return false;
        }
        ++num_PCBs;
    }

    // Calculate values for result
    result->average_waiting_time = (float)total_wait_time / num_PCBs;
    result->average_turnaround_time = (float)time / num_PCBs;
    result->total_run_time = time;

    //Successfully scheduled
    return true;
}

bool shortest_job_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
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

    dyn_array_t *waiting_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);

    //While there are still unscheduled pcbs
    while (!dyn_array_empty(ready_queue) || !dyn_array_empty(waiting_queue)) {
        // While ready queue is not empty and pcb has arrived
        while (!dyn_array_empty(ready_queue) && ((ProcessControlBlock_t*)dyn_array_front(ready_queue))->arrival <= time){
            ProcessControlBlock_t *wait_pcb = dyn_array_front(ready_queue);
            if (wait_pcb == NULL) {
                return false;
            }

            //Push to the wait queue
            if (!dyn_array_push_back(waiting_queue, wait_pcb)) {
                // error with push
                return false;
            }
            //Pop from the ready queue
            if (!dyn_array_pop_front(ready_queue)) {
                // error with pop
                return false;
            }
        }

        if (!dyn_array_empty(waiting_queue)) {
            // Sort waiting queue by burst time
            if (!dyn_array_sort(waiting_queue, compare_by_burst_time)) {
                // Error while sorting
                printf("1");
                dyn_array_destroy(waiting_queue);
                return false;
            }
        }

        //If there are waiting PCBs run one then jump back to the top to check for the next shortest available job
        if(!dyn_array_empty(waiting_queue)) {
            // Get the next pcb to schedule
            ProcessControlBlock_t *current_pcb = dyn_array_front(waiting_queue);
            if (current_pcb == NULL) {
                //error with dyn_array_front
                return false;
            }
            // time when the PCB starts running minus arrival time = that PCBs wait time
            total_wait_time += time - current_pcb->arrival;

            // While there is still burst time run the PCB on the virtual cpu
            while (current_pcb->remaining_burst_time > 0) {
                virtual_cpu(current_pcb);
                current_pcb->started = true;
                ++time;
            }
            // Pop finished PCB
            if (!dyn_array_pop_front(waiting_queue)) {
                // error with pop
                return false;
            }
            ++num_PCBs;
        }
    }

    // Calculate values for result
    result->average_waiting_time = (float)total_wait_time / num_PCBs;
    result->average_turnaround_time = (float)time / num_PCBs;
    result->total_run_time = time;

    //Successfully scheduled
    return true;  
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

    dyn_array_t *waiting_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);

    uint32_t current_cycle = 0;

    /* Loop through until the waiting queue and ready queue are empty */
    while(!dyn_array_empty(waiting_queue) || !dyn_array_empty(ready_queue)){
        
        /* Moves all of the pcbs with the current arrival time to the back of the waiting queue */
        while(dyn_array_front(ready_queue)->arrival == current_cycle){
            waiting_queue.dyn_array_push_back(ready_queue.dyn_array_pop_front());
        }

        /* Sorts waiting queue by burst time */
        if(sizeof(waiting_queue) > 1){
            dyn_array_sort(waiting_queue, compare_by_burst_time);
        }

        /* Gives one cycle of the CPU to the front of the waiting queue which has the shortest burst time */
        virtual_cpu(dyn_array_front(waiting_queue));
        ++time;

        /* Remove the front of the waiting queue if its burst time is complete */
        if(dyn_array_front(waiting_queue)->burst_time < 1){
            if(!dyn_array_pop_front(waiting_queue)) return false;
            num_PCBs++;
        }
    }

    /* Calculate values for result */
    result->average_waiting_time = (float)total_wait_time / num_PCBs;
    result->average_turnaround_time = (float)time / num_PCBs;
    result->total_run_time = time;
    return true;
}
