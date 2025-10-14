////////////////////////////////////////////////////////////////////////////////
// Include files
//

#include <stdio.h>
#include <windows.h>
#include <winternl.h>

//
// End of Include files
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Directives
//

#pragma once
#define STATUS_OK               0
#define STATUS_ERROR            -1
#define SYSCALL_FUNCTIONS_COUNT 500

//
// End of Directives
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Types definition
//

typedef unsigned __int64 QWORD, *PQWORD;

typedef struct
{
    char* functionName;
    QWORD functionAddress;
} function_info_t;

typedef struct _TRIMMED_PEB_LDR_DATA
{
    QWORD      irrelevantData[2];
    LIST_ENTRY inLoadOrderModuleList;
} TRIMMED_PEB_LDR_DATA, *PTRIMMED_PEB_LDR_DATA;

typedef struct _TRIMMED_LDR_DATA_TABLE_ENTRY
{
    LIST_ENTRY     inLoadOrderLinks;
    LIST_ENTRY     inMemoryOrderLinks;
    LIST_ENTRY     inInitializationOrderLinks;
    PVOID          dllBase;
    PVOID          entryPoint;
    ULONG          sizeOfImage;
    UNICODE_STRING fullDllName;
    UNICODE_STRING baseDllName;
} TRIMMED_LDR_DATA_TABLE_ENTRY, *PTRIMMED_LDR_DATA_TABLE_ENTRY;

//
// End of Types definition
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Functions declaration
//

static HANDLE GetNtdllBaseAddress();

static void SetSyscallNumber(
    function_info_t* p_functionList,
    char*            functionName
);

static void SetSyscallAddress(
    function_info_t* p_functionList,
    char*            functionName
);

static int GetExportedNtFunctions(
    HMODULE          ntdllHandle,
    function_info_t* p_functionList,
    DWORD            functionListSize
);

static PIMAGE_EXPORT_DIRECTORY GetImageExportDirectory(HANDLE p_ntdllHandle);

//
// End of Functions declaration
////////////////////////////////////////////////////////////////////////////////
