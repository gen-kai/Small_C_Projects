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
static bool ReallocateInputBuffer(user_input* p_userInput);
static bool AllocateInputBuffer(user_input* p_userInput, u_int desiredInputbufferSize);

int main(u_int argCount, char* argValues[])
{
    if (argCount != 2)
    {
        printf("Usage: main.exe filename.ext");
        return 1;
    }

    FILE* p_inputFile = fopen(argValues[1], "rb");
    if (p_inputFile == NULL)
    {
        printf("Couldn't open input file!\n");
        return 2;
    }


    user_input p_userInput;


    p_userInput.currentInputBufferSize = 0;


    if (!AllocateInputBuffer(&p_userInput, INITIAL_SIZE))
    {
        fclose(p_inputFile);
        return 4;
    }


    if (!GetUserInput(p_inputFile, &p_userInput))
    {
        if (p_userInput.p_inputBuffer != NULL)
        {
            free(p_userInput.p_inputBuffer);
            p_userInput.p_inputBuffer = NULL;
        }

        fclose(p_inputFile);
        return 5;
    }


    printf("Your input: %s\n", p_userInput.p_inputBuffer);
    printf("Your input's length: %ju\n", strnlen(p_userInput.p_inputBuffer, p_userInput.currentInputBufferSize));
    printf("Input buffer length: %u\n", p_userInput.currentInputBufferSize);


    if (p_userInput.p_inputBuffer != NULL)
    {
        free(p_userInput.p_inputBuffer);
        p_userInput.p_inputBuffer = NULL;
    }


    fclose(p_inputFile);
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
                free(p_userInput->p_inputBuffer);
                p_userInput->p_inputBuffer = NULL;


                printf("Couldn't allocate bigger buffer of size %u for user input!\n", p_userInput->currentInputBufferSize);
                return false;
            }


            p_userInput->p_inputBuffer = p_tmpBuffer;
            p_userInput->currentInputBufferSize = desiredInputBufferSize;
        }
    }
    

    return true;
}