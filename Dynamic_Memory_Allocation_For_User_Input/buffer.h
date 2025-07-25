#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define INITIAL_SIZE 2000U
#define MAX_UINT32_TO_STR_BASE10_COUNT 10U

typedef struct
{
    uint32_t inputBufferSize;
    uint32_t inputBufferCapacity;
    char* p_inputBuffer;
}
user_input;

user_input CreateUserInput(uint32_t desiredBufferSize);
bool AllocateInputBuffer(user_input* p_userInput, uint32_t desiredInputbufferSize);
bool ReallocateInputBuffer(user_input* p_userInput);
bool SetBufferChar(char charToSet, uint32_t charIndex, user_input* p_userInput);
void FreeAllocatedMemory(FILE* p_inputFile, user_input* p_userInput);