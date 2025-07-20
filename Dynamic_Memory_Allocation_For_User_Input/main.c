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
    char *p_userInput;
}
user_input;

static bool GetUserInput(FILE *p_inputFile, user_input **userInput);
static bool ReallocateBiggerBuffer(user_input **userInput);
static bool AllocateInputBuffer(user_input **userInput);

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


    user_input *userInput = calloc(1, sizeof(user_input));
    if (userInput == NULL)
    {
        fclose(p_inputFile);
        printf("Couldn't allocate buffer for user input!\n");
        return 3;
    }


    userInput->inputSize = INITIAL_SIZE;


    if (!AllocateInputBuffer(&userInput))
    {
        fclose(p_inputFile);
        free(userInput);
        userInput = NULL;


        return 4;
    }


    if (!GetUserInput(p_inputFile, &userInput))
    {
        if (userInput->p_userInput != NULL)
        {
            free(userInput->p_userInput);
            userInput->p_userInput = NULL;
        }
        
        fclose(p_inputFile);
        free(userInput);
        userInput = NULL;


        return 5;
    }


    printf("Your input: %s\n", userInput->p_userInput);
    printf("Your input's length: %ju\n", strlen(userInput->p_userInput));
    printf("Input buffer length: %u\n", userInput->inputSize);


    if (userInput->p_userInput != NULL)
    {
        free(userInput->p_userInput);
        userInput->p_userInput = NULL;
    }


    fclose(p_inputFile);
    free(userInput);
    userInput = NULL;


    return 0;
}

static bool GetUserInput(FILE *p_inputFile, user_input **userInput)
{
    char inputChar = getc(p_inputFile);
    u_int charIterator = 0;

    while (inputChar != EOF)
    {
        if (charIterator < (*userInput)->inputSize)
        {
            (*userInput)->p_userInput[charIterator] = inputChar;
        }
        else
        {
            if (!ReallocateBiggerBuffer(userInput))
            {
                return false;
            }


            (*userInput)->p_userInput[charIterator] = inputChar;
        }


        inputChar = getc(p_inputFile);
        charIterator++;
    }


    if (charIterator < (*userInput)->inputSize)
    {
        (*userInput)->p_userInput[charIterator] = '\0';
    }
    else
    {
        if (!ReallocateBiggerBuffer(userInput))
        {
            return false;
        }


        (*userInput)->p_userInput[charIterator] = '\0';
    }


    return true;
}

static bool ReallocateBiggerBuffer(user_input **userInput)
{
    if ((*userInput)->inputSize >= INITIAL_SIZE
        && (*userInput)->inputSize < INITIAL_SIZE * 4)
    {
        (*userInput)->inputSize *= 2;
        if (!AllocateInputBuffer(userInput))
        {
            return false;
        }
    }
    else if ((*userInput)->inputSize >= INITIAL_SIZE * 4
             && (*userInput)->inputSize < INITIAL_SIZE * 8)
    {
        (*userInput)->inputSize *= 2;
        if (!AllocateInputBuffer(userInput))
        {
            return false;
        }
    }
    else if ((*userInput)->inputSize >= INITIAL_SIZE * 8
             && (*userInput)->inputSize < INITIAL_SIZE * 16)
    {
        (*userInput)->inputSize *= 1.5;
        if (!AllocateInputBuffer(userInput))
        {
            return false;
        }
    }
    else
    {
        (*userInput)->inputSize *= 1.2;
        if (!AllocateInputBuffer(userInput))
        {
            return false;
        }
    }


    return true;
}

static bool AllocateInputBuffer(user_input **userInput)
{
    char *p_tmpBuffer = (*userInput)->p_userInput;

    if ((*userInput)->inputSize == INITIAL_SIZE)
    {
        (*userInput)->p_userInput = calloc(1, INITIAL_SIZE);
        if ((*userInput)->p_userInput == NULL)
        {
            printf("Couldn't allocate buffer for user input!\n");
            return false;
        }
    }
    else
    {
        p_tmpBuffer = realloc(p_tmpBuffer, (*userInput)->inputSize);
        if (p_tmpBuffer == NULL)
        {
            free((*userInput)->p_userInput);
            (*userInput)->p_userInput = NULL;


            printf("Couldn't allocate bigger buffer of size %u for user input!\n", (*userInput)->inputSize);
            return false;
        }


        (*userInput)->p_userInput = p_tmpBuffer;
    }


    return true;
}