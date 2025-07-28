#include <windows.h>
#include <winternl.h>
#include "syscalls.h"

int main(int argCount, char* argValues[])
{
    HANDLE fileHandle;
    ACCESS_MASK desiredAccess = FILE_GENERIC_WRITE;

    OBJECT_ATTRIBUTES objectAttributes = {0};
    UNICODE_STRING fileName = {0};

    HMODULE ntdllHandle = GetModuleHandle(L"ntdll.dll");
    if (ntdllHandle == NULL)
    {
        return 1;
    }

    FARPROC RtlInitUnicodeString = GetProcAddress(ntdllHandle, "RtlInitUnicodeString");
    if (RtlInitUnicodeString == NULL)
    {
        return 2;
    }

    RtlInitUnicodeString(&fileName, L"\\??\\c:\\Users\\User\\Documents\\GitHub\\Small_C_Projects\\test.txt");
    if (fileName.Length == 0)
    {
        return 3;
    }

    InitializeObjectAttributes(&objectAttributes, &fileName, OBJ_CASE_INSENSITIVE, NULL, NULL);

    IO_STATUS_BLOCK ioStatusBlock = {0};
    LARGE_INTEGER allocationSize = {0};

    ULONG fileAttributes = FILE_ATTRIBUTE_NORMAL;
    ULONG shareAccess = FILE_SHARE_WRITE;
    ULONG createDisposition = FILE_OVERWRITE_IF;
    ULONG createOptions = FILE_SYNCHRONOUS_IO_NONALERT;
    PVOID p_eaBuffer = NULL;
    ULONG eaLength = 0;


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
        eaLength);


    return syscallStatus;
}