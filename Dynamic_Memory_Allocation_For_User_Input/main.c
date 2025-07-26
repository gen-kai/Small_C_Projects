#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"

static bool GetUserInput(FILE* p_inputFile, user_input* p_userInput);

int main(uint32_t argCount, char* argValues[])
{
    if (argCount != 2)
    {
        printf("Usage: main.exe filename.ext");
        return 1;
    }


    user_input userInput = CreateUserInput(INITIAL_SIZE);
    if (!userInput.isBufferAllocated)
    {
        printf("Couldn't create buffer structure!\n");
        FreeAllocatedMemory(NULL, &userInput);
        return 2;
    }

    FILE* p_inputFile = fopen(argValues[1], "rb");
    if (p_inputFile == NULL)
    {
        printf("Couldn't open input file!\n");
        FreeAllocatedMemory(p_inputFile, &userInput);
        return 3;
    }


    if (!GetUserInput(p_inputFile, &userInput))
    {
        FreeAllocatedMemory(p_inputFile, &userInput);
        return 4;
    }


    printf("Your input: %s\n", userInput.p_inputBuffer);
    printf("Your input's length: %ju\n",
           strnlen(userInput.p_inputBuffer, userInput.inputBufferSize));
    printf("Input buffer length: %u\n", userInput.inputBufferSize);


    FreeAllocatedMemory(p_inputFile, &userInput);
    return 0;
}

static bool GetUserInput(FILE* p_inputFile, user_input* p_userInput)
{
    char inputChar = getc(p_inputFile);

    while (inputChar != EOF)
    {
        if (!AppendBuffer(inputChar, p_userInput))
        {
            return false;
        }


        inputChar = getc(p_inputFile);
    }

    if (!AppendBuffer('\0', p_userInput))
    {
        return false;
    }


    return true;
}