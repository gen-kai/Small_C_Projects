# Small_C_Projects
- [Overview](#overview)
- [Dynamic_Memory_Allocation_For_User_Input](#dynamic_memory_allocation_for_user_input)
- [NtCreateFile_Direct_Syscall](#ntcreatefile_direct_syscall)
- [NtCreateFile_Indirect_Syscall_Dbghelp](#ntcreatefile_direct_syscall_dbghelp)
- [Simple_TCP_Echo_Server](#simple_tcp_echo_server)
- [TCP_Echo_Server_WSAEventSelect](#tcp_echo_server_wsaeventselect)
- [TODO](#todo)

## Overview

This repository is a compilation of my various small projects written in C (on/for Windows, using Visual Studio).

## Dynamic_Memory_Allocation_For_User_Input

The code reads file into the buffer, reallocates the buffer if needed and prints text file size and buffer size (bytes).

## NtCreateFile_Direct_Syscall

Small project that implements direct NtCreateFile() syscall from userland. C part is used to prepare and pass parameters to the function that makes a syscall.
MASM part is used to make a syscall.

## NtCreateFile_Indirect_Syscall_Dbghelp

Small project that implements indirect NtCreateFile() syscall from userland. C part is used to catalogue Nt* functions exported by the ntdll.dll (with the help of dbghelp.dll), find syscall number and the address of `syscall` instruction, prepare and pass parameters to the MASM function.
MASM part is used to jump to the address of `syscall` instruction inside ndll.dll.

## Simple_TCP_Echo_Server

Simple echo server that supports 1 simultaneous connection and implemented using blocking sockets (simplest model in Windows).

## TCP_Echo_Server_WSAEventSelect

Echo server that supports up to 63 simultaneous connections and implemented using model based on WSAEventSelect.

## TODO
- TCP_Echo_Server_Completion_Ports
  
  Echo server with the model based on IOCP (I/O Completion Ports)
- NtCreateFile_Indirect_Syscall_Hells_Gate

  Does everything NtCreateFile_Indirect_Syscall_Dbghelp does, but uses Hell's Gate technique instead of relying on dbghelp.dll
- NtCreateFile_Indirect_Syscall_Halos_Gate

  Does everything NtCreateFile_Indirect_Syscall_Hells_Gate does, but uses Halo's Gate technique to retrieve syscall numbers
