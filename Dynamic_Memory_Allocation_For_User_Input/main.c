#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_SIZE 2000U
#define MAX_UINT32_TO_STR_BASE10_COUNT 10U

typedef unsigned int u_int;

typedef struct
{
    u_int inputSize;
    char *p_inputBuffer;
}
user_input;

static bool GetUserInput(FILE *p_inputFile, user_input **p_p_userInput);
static bool ReallocateBiggerBuffer(user_input **p_p_userInput);
static bool AllocateInputBuffer(user_input **p_p_userInput);

int main(u_int argCount, char *argValues[])
{
    if (argCount != 2)
    {
        printf("Usage: main.exe filename.ext");
        return 1;
    }
    
    FILE *p_inputFile = fopen(argValues[1], "rb");
    if (p_inputFile == NULL)
    {
        printf("Couldn't open input file!\n");
        return 2;
    }


    user_input *p_userInput = calloc(1, sizeof(user_input));
    if (p_userInput == NULL)
    {
        fclose(p_inputFile);
        printf("Couldn't allocate buffer for user input!\n");
        return 3;
    }


    p_userInput->inputSize = INITIAL_SIZE;


    if (!AllocateInputBuffer(&p_userInput))
    {
        fclose(p_inputFile);
        free(p_userInput);
        p_userInput = NULL;


        return 4;
    }


    if (!GetUserInput(p_inputFile, &p_userInput))
    {
        if (p_userInput->p_inputBuffer != NULL)
        {
            free(p_userInput->p_inputBuffer);
            p_userInput->p_inputBuffer = NULL;
        }
        
        fclose(p_inputFile);
        free(p_userInput);
        p_userInput = NULL;


        return 5;
    }


    printf("Your input: %s\n", p_userInput->p_inputBuffer);
    printf("Your input's length: %ju\n", strlen(p_userInput->p_inputBuffer));
    printf("Input buffer length: %u\n", p_userInput->inputSize);


    if (p_userInput->p_inputBuffer != NULL)
    {
        free(p_userInput->p_inputBuffer);
        p_userInput->p_inputBuffer = NULL;
    }


    fclose(p_inputFile);
    free(p_userInput);
    p_userInput = NULL;


    return 0;
}

static bool GetUserInput(FILE *p_inputFile, user_input **p_p_userInput)
{
    char inputChar = getc(p_inputFile);
    u_int charIterator = 0;

    while (inputChar != EOF)
    {
        if (charIterator < (*p_p_userInput)->inputSize)
        {
            (*p_p_userInput)->p_inputBuffer[charIterator] = inputChar;
        }
        else
        {
            if (!ReallocateBiggerBuffer(p_p_userInput))
            {
                return false;
            }


            (*p_p_userInput)->p_inputBuffer[charIterator] = inputChar;
        }


        inputChar = getc(p_inputFile);
        charIterator++;
    }


    if (charIterator < (*p_p_userInput)->inputSize)
    {
        (*p_p_userInput)->p_inputBuffer[charIterator] = '\0';
    }
    else
    {
        if (!ReallocateBiggerBuffer(p_p_userInput))
        {
            return false;
        }


        (*p_p_userInput)->p_inputBuffer[charIterator] = '\0';
    }


    return true;
}

static bool ReallocateBiggerBuffer(user_input **p_p_userInput)
{
    if ((*p_p_userInput)->inputSize >= INITIAL_SIZE
        && (*p_p_userInput)->inputSize < INITIAL_SIZE * 4)
    {
        (*p_p_userInput)->inputSize *= 2;
        if (!AllocateInputBuffer(p_p_userInput))
        {
            return false;
        }
    }
    else if ((*p_p_userInput)->inputSize >= INITIAL_SIZE * 4
             && (*p_p_userInput)->inputSize < INITIAL_SIZE * 8)
    {
        (*p_p_userInput)->inputSize *= 2;
        if (!AllocateInputBuffer(p_p_userInput))
        {
            return false;
        }
    }
    else if ((*p_p_userInput)->inputSize >= INITIAL_SIZE * 8
             && (*p_p_userInput)->inputSize < INITIAL_SIZE * 16)
    {
        (*p_p_userInput)->inputSize *= 1.5;
        if (!AllocateInputBuffer(p_p_userInput))
        {
            return false;
        }
    }
    else
    {
        (*p_p_userInput)->inputSize *= 1.2;
        if (!AllocateInputBuffer(p_p_userInput))
        {
            return false;
        }
    }


    return true;
}

static bool AllocateInputBuffer(user_input **p_p_userInput)
{
    char *p_tmpBuffer = (*p_p_userInput)->p_inputBuffer;

    if ((*p_p_userInput)->inputSize == INITIAL_SIZE)
    {
        (*p_p_userInput)->p_inputBuffer = calloc(1, INITIAL_SIZE);
        if ((*p_p_userInput)->p_inputBuffer == NULL)
        {
            printf("Couldn't allocate buffer for user input!\n");
            return false;
        }
    }
    else
    {
        p_tmpBuffer = realloc(p_tmpBuffer, (*p_p_userInput)->inputSize);
        if (p_tmpBuffer == NULL)
        {
            free((*p_p_userInput)->p_inputBuffer);
            (*p_p_userInput)->p_inputBuffer = NULL;


            printf("Couldn't allocate bigger buffer of size %u for user input!\n", (*p_p_userInput)->inputSize);
            return false;
        }


        (*p_p_userInput)->p_inputBuffer = p_tmpBuffer;
    }


    return true;
}