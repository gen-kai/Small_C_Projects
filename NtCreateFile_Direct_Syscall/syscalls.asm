.code

SyscallNtCreateFile proc
    mov r10, rcx
    mov eax, 55h ; syscall number of NtCreateFile
    syscall
    ret
SyscallNtCreateFile endp

end