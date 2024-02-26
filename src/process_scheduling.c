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
    size_t turn_around_time = 0;

    // While ready queue is not empty keep going
    while (!dyn_array_empty(ready_queue)){
        // Get the next pcb
        ProcessControlBlock_t *current_pcb = dyn_array_front(ready_queue);
        if (current_pcb == NULL) {
            //error with dyn_array_front
            return false;
        }
        
        // While there is still burst time on the first PCB run the PCB on the virtual cpu
        while (current_pcb->remaining_burst_time > 0) {
            virtual_cpu(current_pcb);
            current_pcb->started = true;
            ++time;
            total_wait_time += dyn_array_size(ready_queue) - 1;
        }

        // time when the PCB finishes running minus arrival time = that PCBs wait time
        turn_around_time += time - current_pcb->arrival;

        // Pop finished PCB
        if (!dyn_array_pop_front(ready_queue)) {
            // error with pop
            return false;
        }
        ++num_PCBs;    
    }

    // Calculate values for result
    result->average_waiting_time = (float)total_wait_time / num_PCBs;
    result->average_turnaround_time = (float)turn_around_time / num_PCBs;
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
    result->average_turnaround_time = (float)(total_wait_time + time) / num_PCBs;
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

bool round_robin(dyn_array_t *ready_queue, ScheduleResult_t *result, size_t quantum) {
    if (!ready_queue || !result || quantum == 0) {
        return false; // Error handling for invalid input
    }

    unsigned long current_time = 0;
    unsigned long total_waiting_time = 0;
    unsigned long total_turnaround_time = 0;
    int completed_processes = 0;

    while (!dyn_array_empty(ready_queue)) {
        ProcessControlBlock_t *pcb = (ProcessControlBlock_t*)dyn_array_front(ready_queue);
        if (pcb == NULL) {
            return false; // Error handling for null PCB
        }

        // Calculate the actual time slice given to this process
        unsigned long time_slice = (pcb->remaining_burst_time <= quantum) ? pcb->remaining_burst_time : quantum;

        if (!pcb->started) {
            pcb->started = true;
            pcb->start_time = current_time; // Record the start time of the process
        }

        // Update the current time and remaining burst time of the process
        current_time += time_slice;
        pcb->remaining_burst_time -= time_slice;

        if (pcb->remaining_burst_time == 0) {
            // Process completes its execution
            unsigned long turnaround_time = current_time - pcb->arrival;
            total_turnaround_time += turnaround_time;
            total_waiting_time += pcb->start_time - pcb->arrival; // Waiting time is the start time minus the arrival time
            completed_processes++;

            // Remove the completed process from the queue
            dyn_array_pop_front(ready_queue);
        } else {
            // If process does not complete, move it to the back of the queue
            dyn_array_push_back(ready_queue, pcb);
            dyn_array_pop_front(ready_queue);
        }
    }

    // Compute the averages and total run time for the result
    if (completed_processes > 0) {
        result->average_waiting_time = (float)total_waiting_time / completed_processes;
        result->average_turnaround_time = (float)total_turnaround_time / completed_processes;
    } else {
        // No processes were completed
        result->average_waiting_time = 0.0f;
        result->average_turnaround_time = 0.0f;
    }
    result->total_run_time = current_time;

    return true;
}

dyn_array_t *load_process_control_blocks(const char *input_file) 
{
    // Open the file for reading in binary mode
    FILE *file = fopen(input_file, "rb");
    if (!file) {
        return NULL; // Error opening file
    }

    uint32_t num_blocks;
    if(fread(&num_blocks, sizeof(uint32_t), 1, file) != 1){
        fclose(file);
        return NULL; //Error reading from file
    }

    // Create a dynamic array to hold the ProcessControlBlock_t elements
    dyn_array_t *pcb_array = dyn_array_create(num_blocks, sizeof(ProcessControlBlock_t), NULL);
    if (!pcb_array) {
        fclose(file); // Clean up and exit if dynamic array creation failed
        return NULL;
    }

    for(uint32_t i = 0; i < num_blocks; i++){
        uint32_t burst_time;
        uint32_t priority;
        uint32_t arrival_time;

        ProcessControlBlock_t new_pcb = {0, 0, 0, false};
        fread(&burst_time, sizeof(uint32_t), 1, file);
        fread(&priority, sizeof(uint32_t), 1, file);
        fread(&arrival_time, sizeof(uint32_t), 1, file);

        new_pcb.remaining_burst_time = burst_time;
        new_pcb.priority = priority;
        new_pcb.arrival = arrival_time;

        // Insert the PCB into the dynamic array
        if (!dyn_array_push_back(pcb_array, &new_pcb)) {
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
    /* Check for NULL parameters */
    if (ready_queue == NULL || result == NULL) {
        return false;
    }
    /* Sort ready_queue by arrival time */
    if (!dyn_array_sort(ready_queue, compare_by_arrival)) {
        // Error while sorting
        return false;
    }
    
    /* Store values for the schedule result stats */
    unsigned long time = 0;
    size_t num_PCBs = 0;
    size_t total_wait_time = 0;

    dyn_array_t *waiting_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);

    uint32_t current_cycle = 0;

    /* Loop through until the waiting queue and ready queue are empty */
    while(!dyn_array_empty(waiting_queue) || !dyn_array_empty(ready_queue)){
        
        /* Checks if the ready_queue is empty */
        if(dyn_array_size(ready_queue) > 0){
            
            /* The front pcb of the ready_queue */
            ProcessControlBlock_t *potential_pcb = dyn_array_front(ready_queue);

            /* If ready queue has something and the front pcb has the arrival time of the current cycle */
            while(potential_pcb->arrival == current_cycle && dyn_array_size(ready_queue) > 0){

                /* Push the pcb from the front of the ready queue to the back of the waiting queue*/
                dyn_array_push_back(waiting_queue, potential_pcb);

                /* Remove the front pcb from the ready queue*/
                dyn_array_erase(ready_queue, 0);

                /* If the ready queue still has more blocks, check the next block's arrival time */
                if(dyn_array_size(ready_queue) > 0){
                    potential_pcb = dyn_array_front(ready_queue);
                } 
            }
        }

        /* Sorts waiting queue by burst time */
        if(sizeof(waiting_queue) > 1){
            dyn_array_sort(waiting_queue, compare_by_burst_time);
        }

        ProcessControlBlock_t *cur_pcb = dyn_array_front(waiting_queue);

        /* Gives one cycle of the CPU to the front of the waiting queue which has the shortest burst time */
        virtual_cpu(cur_pcb);
        ++time;

        /* Remove the front of the waiting queue if its burst time is complete */
        if(cur_pcb->remaining_burst_time < 1){
            dyn_array_erase(waiting_queue, 0);
            num_PCBs++;
        }

        /* All of the blocks in the waiting queue are waiting besides the first block 
            So, total wait time adds the number of blocks waiting minus 1 */
        total_wait_time += dyn_array_size(waiting_queue) - 1;

        /* Increment cycle for arrival times */
        current_cycle++;
    }

    /* Calculate values for result */
    result->average_waiting_time = (float)total_wait_time / num_PCBs;
    result->average_turnaround_time = (float)(total_wait_time + time) / num_PCBs;
    result->total_run_time = time;
    
    /* Successfully Scheduled */
    return true;
}
