# Small_C_Projects
- [Overview](#overview)
- [Dynamic_Memory_Allocation_For_User_Input](#dynamic_memory_allocation_for_user_input)
- [NtCreateFile_Direct_Syscall](#ntcreatefile_direct_syscall)
- [Simple_TCP_Echo_Server](#simple_tcp_echo_server)
- [TCP_Echo_Server_WSAEventSelect](#tcp_echo_server_wsaeventselect)
- [TODO](#todo)

## Overview

This repository is a compilation of my various small projects written in C (on/for Windows, using Visual Studio).

## Dynamic_Memory_Allocation_For_User_Input

The code reads file into the buffer, reallocates the buffer if needed and prints text file size and buffer size (bytes).

## NtCreateFile_Direct_Syscall

Small project that implements Direct NtCreateFile() Syscall from userland. C part is used to prepare and pass parameters to the function that makes a syscall, MASM part is used to make a syscall.

## Simple_TCP_Echo_Server

Simple echo server that supports 1 simultaneous connection and implemented using blocking sockets (simplest model in Windows).

## TCP_Echo_Server_WSAEventSelect

Echo server that supports up to 63 simultaneous connections and implemented using model based on WSAEventSelect.

## TODO
- TCP_Echo_Server_Completion_Ports
  
  Echo server with the model based on IOCP (I/O Completion Ports)
