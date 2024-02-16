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

int main(int argc, char **argv) 
{
    ::testing::InitGoogleTest(&argc, argv);
    //::testing::AddGlobalTestEnvironment(new GradeEnvironment);
    return RUN_ALL_TESTS();
}
