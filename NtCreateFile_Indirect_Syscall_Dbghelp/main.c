////////////////////////////////////////////////////////////////////////////////
// Description:
//
// This program implements indirect NtCreateFile syscall.
// Actions:
// - Get the handle of ntdll.dll
// - Get the Nt* functions exported by ntdll.dll
//   - Load the dbghelp.dll
//   - Get the NT header of ntdll.dll (with the help of dbghelp.dll)
//   - Get the export directory of ntdll.dll (with the help of dbghelp.dll)
//   - Get addresses of exported function names list and their addresses list
//   (with the help of dbghelp.dll)
//   - Traverse the list of function names and save the Nt* functions' names and
//   addresses to our array of structures (with the help of dbghelp.dll)
// - Get the syscall number of NtCreateFile
//   - Traverse the list of structures and compare the function name with the
//   provided (NtCreateFile)
//   - Once NtCreateFile structure is found, go to the function address and
//   extract the syscall number
// - Get the address of the syscall instruction inside the ntdll.dll
//   - Traverse the list of structures and compare the function name with the
//   provided (NtCreateFile)
//   - Once NtCreateFile structure is found, go to the function address and
//   extract the syscall instruction address
// - Initialize the RtlInitUnicodeString function
// - Initialize the unicode string containing file name
// - Initialize file's ObjectAttributes
// - Call the MASM function SyscallNtCreateFile() to make an indirect syscall
//
// Inspirations:
// - https://redops.at/en/blog/direct-syscalls-vs-indirect-syscalls
// - https://stackoverflow.com/a/4354755
//
// End of Description
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Include files
//

#include <stdio.h>
#include <windows.h>
#include <winternl.h>

#include <dbghelp.h>

#include "syscalls.h"

//
// End of Include files
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Directives
//

#pragma comment(lib, "dbghelp.lib")


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

//
// End of Types definition
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Global variables declaration/definition
//

DWORD         syscallNumber     = -1;
QWORD         syscallAddress    = 0;
unsigned char syscallStub[]     = {0x0f, 0x05};
unsigned char unhookedSsnStub[] = {0x4c, 0x8b, 0xd1, 0xb8};

//
// End of Global variables declaration/definition
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Functions declaration
//

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

//
// End of Functions declaration
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// main() function
//

int main()
{
    function_info_t*  p_functionList;
    HANDLE            fileHandle;
    ACCESS_MASK       desiredAccess    = FILE_GENERIC_WRITE;
    OBJECT_ATTRIBUTES objectAttributes = {0};
    UNICODE_STRING    fileName         = {0};
    IO_STATUS_BLOCK   ioStatusBlock    = {0};
    LARGE_INTEGER     allocationSize   = {0};

    ULONG fileAttributes    = FILE_ATTRIBUTE_NORMAL;
    ULONG shareAccess       = FILE_SHARE_WRITE;
    ULONG createDisposition = FILE_OVERWRITE_IF;
    ULONG createOptions     = FILE_SYNCHRONOUS_IO_NONALERT;
    PVOID p_eaBuffer        = NULL;
    ULONG eaLength          = 0;


    p_functionList = calloc(SYSCALL_FUNCTIONS_COUNT, sizeof(function_info_t));
    if (p_functionList == NULL)
    {
        return STATUS_ERROR;
    }


    HMODULE ntdllHandle = GetModuleHandle(L"ntdll.dll");
    if (ntdllHandle == NULL)
    {
        printf("Couldn't get ntdll.dll handle!");


        return STATUS_ERROR;
    }


    if (GetExportedNtFunctions(
            ntdllHandle,
            p_functionList,
            SYSCALL_FUNCTIONS_COUNT
        )
        == STATUS_ERROR)
    {
        free(p_functionList);


        return STATUS_ERROR;
    }


    SetSyscallNumber(p_functionList, "NtCreateFile");
    if (syscallNumber == (DWORD)-1)
    {
        printf("Couldn't get syscall number!");


        free(p_functionList);


        return STATUS_ERROR;
    }


    SetSyscallAddress(p_functionList, "NtCreateFile");
    if (syscallAddress == 0)
    {
        printf("Couldn't get syscall address!");


        free(p_functionList);


        return STATUS_ERROR;
    }


    FARPROC RtlInitUnicodeString = GetProcAddress(
        ntdllHandle,
        "RtlInitUnicodeString"
    );
    if (RtlInitUnicodeString == NULL)
    {
        printf("Couldn't find RtlInitUnicodeString address!");


        free(p_functionList);


        return STATUS_ERROR;
    }


    RtlInitUnicodeString(
        &fileName,
        L"\\??\\c:\\Users\\User\\Documents\\GitHub\\Small_C_Projects\\test.txt"
    );
    if (fileName.Length == 0)
    {
        printf("Couldn't initialize file name!");


        free(p_functionList);


        return STATUS_ERROR;
    }


    InitializeObjectAttributes(
        &objectAttributes,
        &fileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
    );


    NTSTATUS syscallStatus = SyscallNtCreateFile(
        &fileHandle,
        desiredAccess,
        &objectAttributes,
        &ioStatusBlock,
        &allocationSize,
        fileAttributes,
        shareAccess,
        createDisposition,
        createOptions,
        p_eaBuffer,
        eaLength
    );


    free(p_functionList);


    return syscallStatus;
}

//
// End of main() function
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// SetSyscallNumber() function
//

// This function selects the function from the function list given the function
// name, gets the address from the structure, searches for the sequence of bytes
// (b8 DWORD) in the range of that address until the syscall sequence (0f 05)
// and sets the global variable syscallNumber to the value of the DWORD after
// the mov instruction
static void SetSyscallNumber(
    function_info_t* p_functionList,
    char*            functionName
)
{
    for (DWORD functionIndex = 0; functionIndex < SYSCALL_FUNCTIONS_COUNT;
         functionIndex++)
    {
        if (strcmp(p_functionList[functionIndex].functionName, functionName)
            == 0)
        {
            QWORD startAddress;
            QWORD endAddress;
            QWORD p_syscallNumber;


            startAddress = p_functionList[functionIndex].functionAddress;

            for (endAddress = startAddress;; endAddress++)
            {
                if ((*(PBYTE)endAddress == syscallStub[0])
                    && (*(PBYTE)(endAddress + 1) == syscallStub[1]))
                {
                    break;
                }
            }


            for (QWORD currentAddress = startAddress;
                 currentAddress < endAddress;
                 currentAddress++)
            {
                if ((*(PBYTE)currentAddress == unhookedSsnStub[0])
                    && (*(PBYTE)(currentAddress + 1) == unhookedSsnStub[1])
                    && (*(PBYTE)(currentAddress + 2) == unhookedSsnStub[2])
                    && (*(PBYTE)(currentAddress + 3) == unhookedSsnStub[3]))
                {
                    syscallNumber = *(PDWORD)(currentAddress + 4);


                    break;
                }
            }


            return;
        }
    }


    syscallNumber = -1;
}

//
// End of SetSyscallNumber() function
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// SetSyscallAddress() function
//

// This function selects the function from the function list given the function
// name, gets the address from the structure, searches for the sequence of bytes
// (0f 05) and sets the global variable syscallAddress to the address of the
// first byte of the sequence
static void SetSyscallAddress(
    function_info_t* p_functionList,
    char*            functionName
)
{
    for (DWORD functionIndex = 0; functionIndex < SYSCALL_FUNCTIONS_COUNT;
         functionIndex++)
    {
        if (strcmp(p_functionList[functionIndex].functionName, functionName)
            == 0)
        {
            QWORD
            currentAddress = p_functionList[functionIndex].functionAddress;

            for (; currentAddress < currentAddress + 0x20; currentAddress++)
            {
                if ((*(PBYTE)currentAddress == syscallStub[0])
                    && (*(PBYTE)(currentAddress + 1) == syscallStub[1]))
                {
                    syscallAddress = currentAddress;


                    return;
                }
            }


            break;
        }
    }


    syscallAddress = 0;
}

//
// End of SetSyscallAddress() function
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// GetExportedNtFunctions() function
//

// This function loads the dbghelp.dll, retrieves the NT header of ntdll.dll
// (with the help of dbghelp.dll), locates the export directory of ntdll.dll
// (with the help of dbghelp.dll), locates the addresses of exported function
// names and their addresses (with the help of dbghelp.dll), traverses the list
// of function names and saves the Nt* functions' names and addresses to our
// array of structures (with the help of dbghelp.dll)
static int GetExportedNtFunctions(
    HMODULE          ntdllHandle,
    function_info_t* p_functionList,
    DWORD            functionListSize
)
{
    IMAGE_EXPORT_DIRECTORY* ntdllExportDir;
    ULONG                   ntdllExportDirSize = 0;


    BOOL symInitializeResult = SymInitialize(GetCurrentProcess(), NULL, TRUE);
    if (!symInitializeResult)
    {
        printf(
            "Couldn't initialize dbghelp.dll. Error code: %u \n",
            GetLastError()
        );


        return STATUS_ERROR;
    }


    PIMAGE_NT_HEADERS ntdllHeader = ImageNtHeader(ntdllHandle);
    if (ntdllHeader == NULL)
    {
        printf("Couldn't get ntdll header. Error code: %u \n", GetLastError());


        SymCleanup(GetCurrentProcess());


        return STATUS_ERROR;
    }


    ntdllExportDir = ImageDirectoryEntryToData(
        ntdllHandle,
        TRUE,
        IMAGE_DIRECTORY_ENTRY_EXPORT,
        &ntdllExportDirSize
    );
    if (ntdllExportDir == NULL)
    {
        printf(
            "Couldn't get ntdll export dir. Error code: %u \n",
            GetLastError()
        );


        SymCleanup(GetCurrentProcess());


        return STATUS_ERROR;
    }


    PDWORD functionNames = ImageRvaToVa(
        ntdllHeader,
        ntdllHandle,
        ntdllExportDir->AddressOfNames,
        NULL
    );
    if (functionNames == NULL)
    {
        printf(
            "Couldn't get function names address. Error code: %u \n",
            GetLastError()
        );


        SymCleanup(GetCurrentProcess());


        return STATUS_ERROR;
    }


    PDWORD functionAddresses = ImageRvaToVa(
        ntdllHeader,
        ntdllHandle,
        ntdllExportDir->AddressOfFunctions,
        NULL
    );
    if (functionAddresses == NULL)
    {
        printf(
            "Couldn't get function addresses address. Error code: %u \n",
            GetLastError()
        );


        SymCleanup(GetCurrentProcess());


        return STATUS_ERROR;
    }


    DWORD ntFunctionCount = 0;

    for (DWORD functionIndex = 0; functionIndex < ntdllExportDir->NumberOfNames;
         functionIndex++)
    {
        char* currentFunctionName = ImageRvaToVa(
            ntdllHeader,
            ntdllHandle,
            functionNames[functionIndex],
            NULL
        );
        if (currentFunctionName == NULL)
        {
            printf(
                "Couldn't get function name for function %d. Error code: %u \n",
                functionNames[functionIndex],
                GetLastError()
            );
        }


        PDWORD currentFunctionAddress = ImageRvaToVa(
            ntdllHeader,
            ntdllHandle,
            functionAddresses[functionIndex + 1],
            NULL
        );
        if (currentFunctionName == NULL)
        {
            printf(
                "Couldn't get function address for function %d. Error code: %u "
                "\n",
                functionIndex,
                GetLastError()
            );


            continue;
        }


        if ((currentFunctionName[0] == 'N')
            && (currentFunctionName[1] == 't')
            && (currentFunctionName[2] != 'd')
            && (ntFunctionCount < functionListSize))
        {
            p_functionList[ntFunctionCount].functionName = currentFunctionName;
            p_functionList[ntFunctionCount].functionAddress = (QWORD)
                currentFunctionAddress;

            ntFunctionCount++;
        }
    }


    SymCleanup(GetCurrentProcess());


    return STATUS_OK;
}

//
// End of GetExportedNtFunctions() function
////////////////////////////////////////////////////////////////////////////////
