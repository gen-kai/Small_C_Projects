#pragma once

////////////////////////////////////////////////////////////////////////////////
// Include files
//

#include <windows.h>
#include <winternl.h>

//
// End of Include files
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Functions declaration
//

extern NTSTATUS __stdcall SyscallNtCreateFile(
    PHANDLE            fileHandle,
    ACCESS_MASK        desiredAccess,
    POBJECT_ATTRIBUTES objectAttributes,
    PIO_STATUS_BLOCK   processIoStatusBlock,
    PLARGE_INTEGER     allocationSize,
    ULONG              fileAttributes,
    ULONG              shareAccess,
    ULONG              createDisposition,
    ULONG              createOptions,
    PVOID              eaBuffer,
    ULONG              eaLength
);

//
// End of Functions declaration
////////////////////////////////////////////////////////////////////////////////
