#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "buffer.h"

user_input CreateUserInput(uint32_t desiredBufferSize)
{
    user_input userInput;

    userInput.inputBufferSize = 0;
    userInput.p_inputBuffer = NULL;

    AllocateInputBuffer(&userInput, INITIAL_SIZE);
    return userInput;
}

bool AllocateInputBuffer(user_input* p_userInput, uint32_t desiredInputBufferSize)
{
    if (p_userInput->inputBufferSize < desiredInputBufferSize)
    {
        if (desiredInputBufferSize == INITIAL_SIZE)
        {
            p_userInput->p_inputBuffer = malloc(INITIAL_SIZE);
            if (p_userInput->p_inputBuffer == NULL)
            {
                p_userInput->isBufferAllocated = false;
                printf("Couldn't allocate buffer for user input!\n");
                return false;
            }


            p_userInput->isBufferAllocated = true;
            p_userInput->inputBufferSize = INITIAL_SIZE;
            p_userInput->inputBufferCapacity = INITIAL_SIZE;
        }
        else
        {
            char* p_tmpBuffer = p_userInput->p_inputBuffer;

            p_tmpBuffer = realloc(p_tmpBuffer, desiredInputBufferSize);
            if (p_tmpBuffer == NULL)
            {
                p_userInput->isBufferAllocated = false;
                printf("Couldn't allocate bigger buffer of size %u for user input!\n",
                       p_userInput->inputBufferSize);
                return false;
            }


            p_userInput->p_inputBuffer = p_tmpBuffer;
            p_userInput->isBufferAllocated = true;
            p_userInput->inputBufferCapacity = desiredInputBufferSize
                - p_userInput->inputBufferSize;
            p_userInput->inputBufferSize = desiredInputBufferSize;
        }
    }


    return true;
}

bool ReallocateInputBuffer(user_input* p_userInput)
{
    if (p_userInput->inputBufferSize >= INITIAL_SIZE
        && p_userInput->inputBufferSize < INITIAL_SIZE * 8)
    {
        if (!AllocateInputBuffer(p_userInput, p_userInput->inputBufferSize * 2))
        {
            return false;
        }
    }
    else if (p_userInput->inputBufferSize >= INITIAL_SIZE * 8
             && p_userInput->inputBufferSize < INITIAL_SIZE * 16)
    {
        if (!AllocateInputBuffer(p_userInput, p_userInput->inputBufferSize * 1.5))
        {
            return false;
        }
    }
    else if (((uint64_t) p_userInput->inputBufferSize) * 1.2 <= INT32_MAX)
    {
        if (!AllocateInputBuffer(p_userInput, p_userInput->inputBufferSize * 1.2))
        {
            return false;
        }
    }
    else
    {
        return false;
    }


    return true;
}

bool AppendBuffer(char charToSet, user_input* p_userInput)
{
    if (p_userInput->inputBufferCapacity != 0)
    {
        p_userInput->p_inputBuffer[p_userInput->inputBufferSize - p_userInput->inputBufferCapacity] = charToSet;
        p_userInput->inputBufferCapacity--;
    }
    else
    {
        if (!ReallocateInputBuffer(p_userInput))
        {
            return false;
        }

        if (p_userInput->inputBufferCapacity != 0)
        {
            p_userInput->p_inputBuffer[p_userInput->inputBufferSize - p_userInput->inputBufferCapacity] = charToSet;
            p_userInput->inputBufferCapacity--;
        }
        else
        {
            return false;
        }
    }


    return true;
}

void FreeAllocatedMemory(FILE* p_inputFile, user_input* p_userInput)
{
    if (p_inputFile != NULL)
    {
        fclose(p_inputFile);
    }


    if (p_userInput->p_inputBuffer != NULL)
    {
        free(p_userInput->p_inputBuffer);
        p_userInput->p_inputBuffer = NULL;
    }
}