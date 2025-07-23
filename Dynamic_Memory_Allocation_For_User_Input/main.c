#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_SIZE 2000U
#define MAX_UINT32_TO_STR_BASE10_COUNT 10U

typedef unsigned int u_int;

typedef struct
{
    u_int currentInputBufferSize;
    char* p_inputBuffer;
}
user_input;

static bool GetUserInput(FILE* p_inputFile, user_input* p_userInput);
static user_input CreateUserInput(desiredBufferSize);
static bool ReallocateInputBuffer(user_input* p_userInput);
static bool AllocateInputBuffer(user_input* p_userInput, u_int desiredInputbufferSize);
static void FreeAllocatedMemory(FILE* p_inputFile, user_input* p_userInput);

int main(u_int argCount, char* argValues[])
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
    printf("Your input's length: %ju\n", strnlen(userInput.p_inputBuffer, userInput.currentInputBufferSize));
    printf("Input buffer length: %u\n", userInput.currentInputBufferSize);


    FreeAllocatedMemory(p_inputFile, &userInput);
    return 0;
}

static bool GetUserInput(FILE* p_inputFile, user_input* p_userInput)
{
    char inputChar = getc(p_inputFile);
    u_int charIterator = 0;

    while (inputChar != EOF)
    {
        if (charIterator < p_userInput->currentInputBufferSize)
        {
            p_userInput->p_inputBuffer[charIterator] = inputChar;
        }
        else
        {
            if (!ReallocateInputBuffer(p_userInput))
            {
                return false;
            }


            p_userInput->p_inputBuffer[charIterator] = inputChar;
        }


        inputChar = getc(p_inputFile);
        charIterator++;
    }


    if (charIterator < p_userInput->currentInputBufferSize)
    {
        p_userInput->p_inputBuffer[charIterator] = '\0';
    }
    else
    {
        if (!ReallocateInputBuffer(p_userInput))
        {
            return false;
        }


        p_userInput->p_inputBuffer[charIterator] = '\0';
    }


    return true;
}

static user_input CreateUserInput(desiredBufferSize)
{
    user_input userInput;

    userInput.currentInputBufferSize = 0;
    userInput.p_inputBuffer = NULL;


    if (!AllocateInputBuffer(&userInput, INITIAL_SIZE))
    {
        FreeAllocatedMemory(NULL, &userInput);
        exit(2);
    }


    return userInput;
}

static bool AllocateInputBuffer(user_input* p_userInput, u_int desiredInputBufferSize)
{
    if (p_userInput->currentInputBufferSize < desiredInputBufferSize)
    {
        if (desiredInputBufferSize == INITIAL_SIZE)
        {
            p_userInput->p_inputBuffer = malloc(INITIAL_SIZE);
            if (p_userInput->p_inputBuffer == NULL)
            {
                printf("Couldn't allocate buffer for user input!\n");
                return false;
            }


            p_userInput->currentInputBufferSize = INITIAL_SIZE;
        }
        else
        {
            char* p_tmpBuffer = p_userInput->p_inputBuffer;

            p_tmpBuffer = realloc(p_tmpBuffer, desiredInputBufferSize);
            if (p_tmpBuffer == NULL)
            {
                printf("Couldn't allocate bigger buffer of size %u for user input!\n", p_userInput->currentInputBufferSize);
                return false;
            }


            p_userInput->p_inputBuffer = p_tmpBuffer;
            p_userInput->currentInputBufferSize = desiredInputBufferSize;
        }
    }


    return true;
}

static bool ReallocateInputBuffer(user_input* p_userInput)
{
    if (p_userInput->currentInputBufferSize >= INITIAL_SIZE
        && p_userInput->currentInputBufferSize < INITIAL_SIZE * 8)
    {
        if (!AllocateInputBuffer(p_userInput, p_userInput->currentInputBufferSize * 2))
        {
            return false;
        }
    }
    else if (p_userInput->currentInputBufferSize >= INITIAL_SIZE * 8
             && p_userInput->currentInputBufferSize < INITIAL_SIZE * 16)
    {
        if (!AllocateInputBuffer(p_userInput, p_userInput->currentInputBufferSize * 1.5))
        {
            return false;
        }
    }
    else
    {
        if (!AllocateInputBuffer(p_userInput, p_userInput->currentInputBufferSize * 1.2))
        {
            return false;
        }
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