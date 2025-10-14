////////////////////////////////////////////////////////////////////////////////
// Description:
//
// This program implements indirect NtCreateFile syscall.
// Actions:
// - Get the handle of ntdll.dll
// - Get the Nt* functions exported by ntdll.dll
//   - Get the NT header of ntdll.dll (from the DOS header)
//   - Get the export directory of ntdll.dll (from the optional NT header)
//   - Get addresses of exported function names list and their addresses list
//   (via the ntdll.dll's EAT)
//   - Traverse the list of function names and save the Nt* functions' names and
//   addresses to our array of structures
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
// Inspirations/Resources:
// - https://redops.at/en/blog/direct-syscalls-vs-indirect-syscalls
// - https://redops.at/en/blog/exploring-hells-gate
// - https://github.com/am0nsec/HellsGate/blob/master/HellsGate/main.c
// -
// https://sandsprite.com/CodeStuff/Understanding_the_Peb_Loader_Data_List.html
// - https://www.vergiliusproject.com/
//
// End of Description
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Include files
//

#include <stdio.h>
#include <windows.h>
#include <winternl.h>

#include "declarations.h"
#include "syscalls.h"

//
// End of Include files
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


    HMODULE ntdllHandle = GetNtdllBaseAddress();
    if (ntdllHandle == NULL)
    {
        free(p_functionList);


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


    FARPROC RtlInitUnicodeString =
        GetProcAddress(ntdllHandle, "RtlInitUnicodeString");
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
// GetNtdllBaseAddress() function
//

// This function finds current thread's TEB, acquires current process' PEB from
// the TEB, finds process' Ldr list entry (which is usually 1st) and ntdll's Ldr
// list entry (which is usually 2nd, after the process executable itself), finds
// the module with the name "ntdll.dll" and returns the module's base address
// (which is a handle to the DLL we can get by calling GetModuleHandle())
static HANDLE GetNtdllBaseAddress()
{
    PTEB p_currentTeb = (PTEB)__readgsqword(0x30);


    PPEB p_currentPeb = p_currentTeb->ProcessEnvironmentBlock;


    PTRIMMED_PEB_LDR_DATA p_currentLdrData =
        (PTRIMMED_PEB_LDR_DATA)p_currentPeb->Ldr;


    PTRIMMED_LDR_DATA_TABLE_ENTRY p_nextModule =
        (PTRIMMED_PEB_LDR_DATA)(p_currentLdrData->inLoadOrderModuleList.Flink);


    while (p_nextModule != NULL)
    {
        if (wcsncmp(
                p_nextModule->baseDllName.Buffer,
                L"ntdll.dll",
                p_nextModule->baseDllName.Length
            )
            == 0)
        {
            return p_nextModule->dllBase;
        }


        p_nextModule =
            (PTRIMMED_LDR_DATA_TABLE_ENTRY)(p_nextModule->inLoadOrderLinks
                                                .Flink);
    }


    return NULL;
}

//
// End of GetNtdllBaseAddress() function
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// GetExportedNtFunctions() function
//

// This function locates the export directory of ntdll.dll
// (GetImageExportDirectory()), locates the addresses of exported function names
// and their addresses, traverses the list of function names and saves the Nt*
// functions' names and addresses to our array of structures
static int GetExportedNtFunctions(
    HMODULE          ntdllHandle,
    function_info_t* p_functionList,
    DWORD            functionListSize
)
{
    IMAGE_EXPORT_DIRECTORY* ntdllExportDir;
    ULONG                   ntdllExportDirSize = 0;
    DWORD                   ntFunctionCount    = 0;


    ntdllExportDir = GetImageExportDirectory(ntdllHandle);
    if (ntdllExportDir == NULL)
    {
        printf(
            "Couldn't get ntdll export dir. Error code: %u \n",
            GetLastError()
        );


        return STATUS_ERROR;
    }


    PDWORD functionNames = (PDWORD)((QWORD)ntdllHandle
                                    + ntdllExportDir->AddressOfNames);


    PDWORD functionAddresses = (PDWORD)((QWORD)ntdllHandle
                                        + ntdllExportDir->AddressOfFunctions);


    for (DWORD functionIndex = 0; functionIndex < ntdllExportDir->NumberOfNames;
         functionIndex++)
    {
        char* currentFunctionName = (char*)((QWORD)ntdllHandle
                                            + functionNames[functionIndex]);


        PQWORD currentFunctionAddress =
            (PDWORD)((QWORD)ntdllHandle + functionAddresses[functionIndex + 1]);


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


    return STATUS_OK;
}

//
// End of GetExportedNtFunctions() function
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// GetImageExportDirectory() function
//

// This function retrieves the address of the module's NT headers from the DOS
// header, gets the relative virtual address of the module's export directory
// and returns its virtual address
static PIMAGE_EXPORT_DIRECTORY GetImageExportDirectory(HANDLE p_ntdllHandle)
{
    PIMAGE_DOS_HEADER p_ntdllDosHeader = p_ntdllHandle;
    if (p_ntdllDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
    {
        return NULL;
    }


    PIMAGE_NT_HEADERS p_ntdllNtHeaders = (QWORD)p_ntdllHandle
                                         + p_ntdllDosHeader->e_lfanew;
    if (p_ntdllNtHeaders->Signature != IMAGE_NT_SIGNATURE)
    {
        return NULL;
    }


    DWORD ntdllExportDirectoryRva =
        p_ntdllNtHeaders->OptionalHeader.DataDirectory[0].VirtualAddress;


    return (PIMAGE_EXPORT_DIRECTORY)((QWORD)p_ntdllHandle
                                     + ntdllExportDirectoryRva);
}

//
// End of GetImageExportDirectory() function
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
