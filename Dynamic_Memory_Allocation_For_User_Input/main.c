#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_SIZE 2000U
#define MAX_UINT32_TO_STR_BASE10_COUNT 10U

typedef struct
{
    uint32_t inputBufferSize;
    uint32_t inputBufferCapacity;
    char* p_inputBuffer;
}
user_input;

static bool GetUserInput(FILE* p_inputFile, user_input* p_userInput);
static user_input CreateUserInput(desiredBufferSize);
static bool AllocateInputBuffer(user_input* p_userInput, uint32_t desiredInputbufferSize);
static bool ReallocateInputBuffer(user_input* p_userInput);
static bool SetBufferChar(char charToSet, uint32_t charIndex,user_input* p_userInput);
static void FreeAllocatedMemory(FILE* p_inputFile, user_input* p_userInput);

int main(uint32_t argCount, char* argValues[])
{
    if (argCount != 2)
    {
        printf("Usage: main.exe filename.ext");
        return 1;
    }


    user_input userInput = CreateUserInput(INITIAL_SIZE);


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
        if (!SetBufferChar(inputChar,
                           p_userInput->inputBufferSize - p_userInput->inputBufferCapacity,
                           p_userInput))
        {
            return false;
        }


        p_userInput->p_inputBuffer[p_userInput->inputBufferSize - p_userInput->inputBufferCapacity] = inputChar;


        inputChar = getc(p_inputFile);
    }

    if (!SetBufferChar('\0',
                       p_userInput->inputBufferSize - p_userInput->inputBufferCapacity,
                       p_userInput))
    {
        return false;
    }


    return true;
}

static user_input CreateUserInput(desiredBufferSize)
{
    user_input userInput;

    userInput.inputBufferSize = 0;
    userInput.p_inputBuffer = NULL;


    if (!AllocateInputBuffer(&userInput, INITIAL_SIZE))
    {
        FreeAllocatedMemory(NULL, &userInput);
        exit(2);
    }


    return userInput;
}

static bool AllocateInputBuffer(user_input* p_userInput, uint32_t desiredInputBufferSize)
{
    if (p_userInput->inputBufferSize < desiredInputBufferSize)
    {
        if (desiredInputBufferSize == INITIAL_SIZE)
        {
            p_userInput->p_inputBuffer = malloc(INITIAL_SIZE);
            if (p_userInput->p_inputBuffer == NULL)
            {
                printf("Couldn't allocate buffer for user input!\n");
                return false;
            }


            p_userInput->inputBufferSize = INITIAL_SIZE;
            p_userInput->inputBufferCapacity = INITIAL_SIZE;
        }
        else
        {
            char* p_tmpBuffer = p_userInput->p_inputBuffer;

            p_tmpBuffer = realloc(p_tmpBuffer, desiredInputBufferSize);
            if (p_tmpBuffer == NULL)
            {
                printf("Couldn't allocate bigger buffer of size %u for user input!\n",
                       p_userInput->inputBufferSize);
                return false;
            }


            p_userInput->p_inputBuffer = p_tmpBuffer;
            p_userInput->inputBufferCapacity = desiredInputBufferSize
                - p_userInput->inputBufferSize;
            p_userInput->inputBufferSize = desiredInputBufferSize;
        }
    }


    return true;
}

static bool ReallocateInputBuffer(user_input* p_userInput)
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

static bool SetBufferChar(char charToSet, uint32_t charIndex, user_input* p_userInput)
{
    if (p_userInput->inputBufferCapacity != 0)
    {
        p_userInput->p_inputBuffer[charIndex] = charToSet;
        p_userInput->inputBufferCapacity--;
    }
    else
    {
        if (!ReallocateInputBuffer(p_userInput))
        {
            return false;
        }


        p_userInput->p_inputBuffer[charIndex] = charToSet;
        p_userInput->inputBufferCapacity--;
    }


    return true;
}

static void FreeAllocatedMemory(FILE* p_inputFile, user_input* p_userInput)
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