#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "buffer.h"

user_input CreateUserInput(uint32_t desiredBufferSize)
{
    user_input userInput;

    userInput.bufferSize    = 0;
    userInput.p_inputBuffer = NULL;

    AllocateInputBuffer(&userInput, INITIAL_SIZE);
    return userInput;
}

bool AllocateInputBuffer(user_input* p_userInput, uint32_t desiredBufferSize)
{
    if (p_userInput->bufferSize < desiredBufferSize)
    {
        if (desiredBufferSize == INITIAL_SIZE)
        {
            p_userInput->p_inputBuffer = malloc(INITIAL_SIZE);
            if (p_userInput->p_inputBuffer == NULL)
            {
                p_userInput->isBufferAllocated = false;
                printf("Couldn't allocate buffer for user input!\n");
                return false;
            }


            p_userInput->isBufferAllocated = true;
            p_userInput->bufferSize        = INITIAL_SIZE;
            p_userInput->bufferCapacity    = INITIAL_SIZE;
        }
        else
        {
            char* p_tmpBuffer = p_userInput->p_inputBuffer;

            p_tmpBuffer = realloc(p_tmpBuffer, desiredBufferSize);
            if (p_tmpBuffer == NULL)
            {
                p_userInput->isBufferAllocated = false;
                printf(
                    "Couldn't allocate bigger buffer of size %u for user "
                    "input!\n",
                    p_userInput->bufferSize
                );
                return false;
            }


            p_userInput->p_inputBuffer     = p_tmpBuffer;
            p_userInput->isBufferAllocated = true;
            p_userInput->bufferCapacity    = desiredBufferSize
                                          - p_userInput->bufferSize;
            p_userInput->bufferSize = desiredBufferSize;
        }
    }


    return true;
}

bool ReallocateInputBuffer(user_input* p_userInput)
{
    if ((p_userInput->bufferSize >= INITIAL_SIZE)
        && (p_userInput->bufferSize < INITIAL_SIZE * 8))
    {
        if (!AllocateInputBuffer(p_userInput, p_userInput->bufferSize * 2))
        {
            return false;
        }
    }
    else if ((p_userInput->bufferSize >= INITIAL_SIZE * 8)
             && (p_userInput->bufferSize < INITIAL_SIZE * 16))
    {
        if (!AllocateInputBuffer(p_userInput, p_userInput->bufferSize * 1.5))
        {
            return false;
        }
    }
    else if (((uint64_t)p_userInput->bufferSize) * 1.2 <= INT32_MAX)
    {
        if (!AllocateInputBuffer(p_userInput, p_userInput->bufferSize * 1.2))
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
    if (p_userInput->bufferCapacity != 0)
    {
        p_userInput->p_inputBuffer
            [p_userInput->bufferSize - p_userInput->bufferCapacity] = charToSet;
        p_userInput->bufferCapacity--;
    }
    else
    {
        if (!ReallocateInputBuffer(p_userInput))
        {
            return false;
        }

        if (p_userInput->bufferCapacity != 0)
        {
            p_userInput->p_inputBuffer
                [p_userInput->bufferSize
                 - p_userInput->bufferCapacity] = charToSet;
            p_userInput->bufferCapacity--;
        }
        else
        {
            return false;
        }
    }


    return true;
}

void FreeInputBuffer(user_input* p_userInput)
{
    if (p_userInput->p_inputBuffer != NULL)
    {
        free(p_userInput->p_inputBuffer);
        p_userInput->p_inputBuffer = NULL;
    }
}
