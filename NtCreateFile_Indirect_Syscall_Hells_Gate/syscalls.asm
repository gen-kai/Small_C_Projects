extern syscallNumber:dword
extern syscallAddress:qword

.code

SyscallNtCreateFile proc
    mov r10, rcx
    mov eax, syscallNumber ; set syscallNumber to a syscall number of NtCreateFile
    jmp qword ptr [syscallAddress]
    ret
SyscallNtCreateFile endp

end