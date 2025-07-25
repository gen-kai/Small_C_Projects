#pragma once

#include <stdbool.h>
#include <stdint.h>

#define INITIAL_SIZE 2000U
#define MAX_UINT32_TO_STR_BASE10_COUNT 10U

typedef struct
{
    bool isBufferAllocated;
    uint32_t bufferSize;
    uint32_t bufferCapacity;
    char* p_inputBuffer;
}
user_input;

user_input CreateUserInput(uint32_t desiredBufferSize);
bool AllocateInputBuffer(user_input* p_userInput, uint32_t desiredBufferSize);
bool ReallocateInputBuffer(user_input* p_userInput);
bool AppendBuffer(char charToSet, user_input* p_userInput);
void FreeInputBuffer(user_input* p_userInput);