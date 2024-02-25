#include <fcntl.h>
#include <stdio.h>
#include "gtest/gtest.h"
#include <pthread.h>
#include "../include/processing_scheduling.h"

// Using a C library requires extern "C" to prevent function managling
extern "C" 
{
#include <dyn_array.h>
}

/*
#define NUM_PCB 30
#define QUANTUM 5 // Used for Robin Round for process as the run time limit

unsigned int score;
unsigned int total;

class GradeEnvironment : public testing::Environment 
{
    public:
        virtual void SetUp() 
        {
            score = 0;
            total = 210;
        }

        virtual void TearDown()
        {
            ::testing::Test::RecordProperty("points_given", score);
            ::testing::Test::RecordProperty("points_total", total);
            std::cout << "SCORE: " << score << '/' << total << std::endl;
        }
};
*/

// Test case 1: Successfully load PCB burst times from a file
TEST(load_process_control_blocks, Success) {
    // Setup: Create a temporary file and write PCB burst times to it
    const char *tempFileName = "temp_pcb_data.bin";
    FILE *tempFile = fopen(tempFileName, "wb");
    ASSERT_TRUE(tempFile != NULL);
    uint32_t burstTimes[] = {100, 200, 300};
    size_t numElements = sizeof(burstTimes) / sizeof(burstTimes[0]);
    ASSERT_EQ(fwrite(burstTimes, sizeof(uint32_t), numElements, tempFile), numElements);
    fclose(tempFile);

    // Test: Call the function with the temporary file
    dyn_array_t *pcbArray = load_process_control_blocks(tempFileName);
    ASSERT_TRUE(pcbArray != NULL);

    // Verify: Ensure the dynamic array contains the correct values
    ASSERT_EQ(dyn_array_size(pcbArray), numElements);
    for (size_t i = 0; i < numElements; ++i) {
        ProcessControlBlock_t pcb;
        ASSERT_TRUE(dyn_array_extract_front(pcbArray, &pcb));
        ASSERT_EQ(pcb.remaining_burst_time, burstTimes[i]);
    }

    // Cleanup: Remove temporary file and free dynamic array
    remove(tempFileName);
    dyn_array_destroy(pcbArray);
}

// Test case 2: Fail to load PCB burst times due to file open error
TEST(load_process_control_blocks, FileOpenError) {
    // Setup: Use a non-existent file name
    const char *invalidFileName = "non_existent_file.bin";

    // Test: Call the function with the invalid file name
    dyn_array_t *pcbArray = load_process_control_blocks(invalidFileName);
    
    // Verify: Function should return NULL
    ASSERT_TRUE(pcbArray == NULL);
}

/*
* Tests for First Come First Serve function
*/

TEST(first_come_first_serve, MultiplePCBs) {
    // Create a ready queue
    dyn_array_t *ready_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
    ScheduleResult_t result;

    // Set up pcbs with different arrival times and burst times
    ProcessControlBlock_t pcb1 = {10, 1, 0, false};
    ProcessControlBlock_t pcb2 = {3, 2, 5, false};
    ProcessControlBlock_t pcb3 = {7, 3, 5, false};

    //Fill the ready queue with the test pcbs
    dyn_array_push_back(ready_queue, &pcb1);
    dyn_array_push_back(ready_queue, &pcb2);
    dyn_array_push_back(ready_queue, &pcb3);

    // run FCFS
    bool test = first_come_first_serve(ready_queue, &result);

    // function succeeded and emptied the queue
    ASSERT_TRUE(test);
    ASSERT_TRUE(dyn_array_empty(ready_queue));

    // Clean up
    dyn_array_destroy(ready_queue);
}

TEST(first_come_first_serve, EmptyQueue) {
    // Create a ready queue
    dyn_array_t *ready_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
    ScheduleResult_t result;
    
    // run FCFS with empty queue
    bool test = first_come_first_serve(ready_queue, &result);

    // function succeeded and queue is still empty
    ASSERT_FALSE(test);
    ASSERT_TRUE(dyn_array_empty(ready_queue));

    // Clean up
    dyn_array_destroy(ready_queue);    
}

TEST(RoundRobinScheduler, SingleProcess) {
    dyn_array_t *ready_queue = dyn_array_create(1, sizeof(ProcessControlBlock_t), NULL);
    ProcessControlBlock_t pcb = {0, 10, 0, false}; // arrival time: 0, burst time: 10, started: false
    dyn_array_push_back(ready_queue, &pcb);
    
    ScheduleResult_t result;
    size_t quantum = 5;
    unsigned long int total = 10;
    
    bool success = round_robin(ready_queue, &result, quantum);
    
    ASSERT_TRUE(success);
    ASSERT_FLOAT_EQ(0.0, result.average_waiting_time); // No waiting time for a single process
    ASSERT_FLOAT_EQ(10.0, result.average_turnaround_time); // Turnaround time equals burst time
    ASSERT_EQ(total, result.total_run_time); // Total run time equals burst time
    
    dyn_array_destroy(ready_queue);
}

TEST(RoundRobinScheduler, MultipleProcesses) {
    dyn_array_t *ready_queue = dyn_array_create(3, sizeof(ProcessControlBlock_t), NULL);
    ProcessControlBlock_t pcb1 = {0, 10, 0, false};
    ProcessControlBlock_t pcb2 = {1, 20, 0, false};
    ProcessControlBlock_t pcb3 = {2, 30, 0, false}; // Processes with different arrival and burst times
    dyn_array_push_back(ready_queue, &pcb1);
    dyn_array_push_back(ready_queue, &pcb2);
    dyn_array_push_back(ready_queue, &pcb3);
    
    ScheduleResult_t result;
    size_t quantum = 10;
    unsigned long int total = 50;
    
    bool success = round_robin(ready_queue, &result, quantum);
    
    
    ASSERT_TRUE(success);

    ASSERT_FLOAT_EQ(9, result.average_waiting_time);
    ASSERT_FLOAT_EQ(29, result.average_turnaround_time);
    ASSERT_EQ(total, result.total_run_time);
    
    dyn_array_destroy(ready_queue);
}

int main(int argc, char **argv) 
{
    ::testing::InitGoogleTest(&argc, argv);
    //::testing::AddGlobalTestEnvironment(new GradeEnvironment);
    return RUN_ALL_TESTS();
}
