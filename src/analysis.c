#include <stdio.h>
#include <stdlib.h>

#include "dyn_array.h"
#include "processing_scheduling.h"

#define FCFS "FCFS"
#define P "P"
#define RR "RR"
#define SJF "SJF"

// Add and comment your analysis code in this function.
// THIS IS NOT FINISHED.
int main(int argc, char **argv) 
{
    if (argc < 3) 
    {
        printf("%s <pcb file> <schedule algorithm> [quantum]\n", argv[0]);
        return EXIT_FAILURE;
    }

    //abort();  // replace me with implementation.
    /* Create Ready Queue */
    dyn_array_t* ready_queue;
    ready_queue = load_process_control_blocks(argv(0));

    ScheduleResult_t* time_data;

    /* Run the First Come First Serve algorithm */
    if(strcmp(argv[1], FCFS) == 0){
        if(!first_come_first_serve(ready_queue, time_data)){
            printf("Scheduling Algorithm Failed");
            return EXIT_FAILURE;
        }
    }

    /* Milestone says to do shortest time remaining first, not priority */
    else if(strcmp(argv[1], P) == 0){
        if(!priority(ready_queue, time_data)){
            printf("Scheduling Algorithm Failed");
            return EXIT_FAILURE;
        }
    }

    /* Run the Round Robin Algorithm */
    else if(strcmp(argv[1], RR) == 0){
        if(!round_robin(ready_queue, time_data)){
            printf("Scheduling Algorithm Failed");
            return EXIT_FAILURE;
        }
    }

    /* Run the Shortest Job First Algorithm */
    else if(strcmp(argv[1], SJF) == 0){
        if(!shortest_job_first(ready_queue, time_data)){
            printf("Scheduling Algorithm Failed");
            return EXIT_FAILURE;
        }
    }

    /* The command line did not give a valid schedule algorithm input */
    else{
        printf("Invalid schedule algorithm");
        return EXIT_FAILURE;
    }
    
    /* Destroy the ready_queue struct */
    dyn_array_destroy(ready_queue);

    FILE* fp;
    fp = fopen("../readme.md", "r");

    if(!fp){
        printf("File Open Error");
        return EXIT_FAILURE;
    }

    fprintf(fp, "Average Waiting Time: %s\n", time_data->average_waiting_time);
    fprintf(fp, "Average Turnaround Time: %s\n", time_data->average_turnaround_time);
    fprintf(fp, "Total Run Time: %s\n", time_data->total_run_time);

    fclose(fp);

    free(time_data);

    return EXIT_SUCCESS;
}
