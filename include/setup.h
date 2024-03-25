#ifndef SETUP_H
#define SETUP_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/user.h>
#include <sys/uio.h>
#include "mutex.h"
#include "mem_array.h"
#include "webserver.h"

#define UNUSED(expr) do { (void) (expr); } while (0)
#define LONG_SIZE sizeof (long)
#define CHAR_SIZE sizeof (char)
#define SHOW_MEM_INTERVAL_uS 5 * 1e6

void Peek_Memory (pid_t pid, uintptr_t addr, char *buf, size_t len) {
    char* buf_tmp = buf;

    if (!pid || !addr || !len) {
        buf [0] = '\0';
        return;
    }

    union u {
            long val;
            char chars [LONG_SIZE];
    } data;

    size_t offset;
    for (offset = 0; offset + LONG_SIZE < len; offset += LONG_SIZE) {
        data.val = ptrace (PTRACE_PEEKDATA, pid, addr + offset, NULL);
        memcpy (buf_tmp, data.chars, LONG_SIZE);
        buf_tmp += LONG_SIZE;
    }
    const size_t rest = len % LONG_SIZE;
    if (rest > 0) {
        data.val = ptrace (PTRACE_PEEKDATA, pid, addr + offset, NULL);
        memcpy (buf_tmp, data.chars, rest);
    }
    // buf [len] = '\0';
}

void Trace (pid_t child, mem_array_t* child_mem, mutex_t* child_mem_mut) {
    int status;
    struct user_regs_struct regs;
    int insyscall = 0;
    mem_t mem;
    int idx_int;

    while (1) {

        wait (&status);
        if (WIFEXITED (status)) break;

        ptrace (PTRACE_GETREGS, child, NULL, &regs);
        switch (regs.orig_rax) {
            case __NR_mmap:
                if (insyscall == 0) {
                    /* Syscall entry */
                    insyscall = 1;
                    printf ("MMAP (%lld) called with addr=0x%llx, len=%lld, prot=%lld   ->   ", regs.orig_rax, regs.rdi, regs.rsi, regs.rdx);
                } else {
                    /* Syscall exit */
                    // save it
                    if (regs.rax > 0) {
                        mem.addr = regs.rax;
                        mem.len = regs.rsi;
                        Acquire_Mutex (child_mem_mut, true);
                        Append_Mem_Array (child_mem, mem);
                        Release_Mutex (child_mem_mut, true);
                    }

                    // print it
                    // char* value = (char*) malloc ((regs.rsi + 1) * CHAR_SIZE);
                    // peek_memory (child, (uintptr_t) regs.rax, value, regs.rsi);
                    printf ("returned addr=0x%llx\n", regs.rax);
                    // for (size_t i = 0; i < regs.rsi; i++) {
                    //     if (value [i] == '\0') break;
                    //     printf ("%x", value [i]);
                    // }
                    // printf (" (str=%s) \n", value);
                    // free (value);

                    insyscall = 0;
                }
                break;

            case __NR_munmap:
                if (insyscall == 0) {
                    /* Syscall entry */
                    insyscall = 1;
                    printf ("MUNMAP (%lld) called with addr=0x%llx, len=%lld   ->   ", regs.orig_rax, regs.rdi, regs.rsi);
                } else {
                    /* Syscall exit */
                    if (regs.rax == 0) {
                        Acquire_Mutex (child_mem_mut, false);
                        idx_int = Find_Addr_Mem_Array (child_mem, regs.rdi);
                        Release_Mutex (child_mem_mut, false);
                        if (idx_int >= 0) {
                            Acquire_Mutex (child_mem_mut, true);
                            Remove_Mem_Array (child_mem, (size_t) idx_int);
                            Release_Mutex (child_mem_mut, true);
                        }
                    }
                    printf ("returned %s (%lld)\n", (regs.rax == 0) ? "success" : "error", regs.rax);
                    insyscall = 0;
                }
                break;

            case __NR_brk:
                if (insyscall == 0) {
                    /* Syscall entry */
                    insyscall = 1;
                    printf ("BRK (%lld) called with addr=0x%llx   ->   ", regs.orig_rax, regs.rdi);
                } else {
                    /* Syscall exit */
                    printf ("returned addr=0x%llx\n", regs.rax);
                    insyscall = 0;
                }
                break;

            case __NR_mprotect:
                if (insyscall == 0) {
                    /* Syscall entry */
                    insyscall = 1;
                    printf ("MPROTECT (%lld) called with addr=0x%llx, len=%lld, prot=%lld   ->   ", regs.orig_rax, regs.rdi, regs.rsi, regs.rdx);
                } else {
                    /* Syscall exit */
                    printf ("returned %s (%lld)\n", (regs.rax == 0) ? "success" : "error", regs.rax);
                    insyscall = 0;
                }
                break;

            default:
                printf ("The child made a system call %lld\n", regs.orig_rax);
                break;
        }

        ptrace (PTRACE_SYSCALL, child, NULL, NULL);

    }

}

struct display_mem_args {
    pid_t child;
    webserver_t* ws;
    mem_array_t* child_mem;
    mutex_t* child_mem_mut;
};

void* Display_Mem (void* args) {
    pid_t child = ((struct display_mem_args*) args)->child;
    webserver_t* ws = ((struct display_mem_args*) args)->ws;
    mem_array_t* child_mem = ((struct display_mem_args*) args)->child_mem;
    mutex_t* child_mem_mut = ((struct display_mem_args*) args)->child_mem_mut;

    struct timespec tic;
    struct timespec tac;
    size_t len;

    while (1) {
        clock_gettime (CLOCK_REALTIME, &tic);

        Acquire_Mutex (child_mem_mut, false);
        char* data = Serialized_Mem_Array (child_mem, &len, child);
        Release_Mutex (child_mem_mut, false);
        if (data) {
            Send_Webserver (ws, data, len);
        }
        free (data);

        clock_gettime (CLOCK_REALTIME, &tac);
        usleep ((__useconds_t) (SHOW_MEM_INTERVAL_uS - (double) (((tac.tv_nsec - tic.tv_nsec) > 0) ? (tac.tv_nsec - tic.tv_nsec) : 0) * 1e-3));
    }

}

#endif  // SETUP_H