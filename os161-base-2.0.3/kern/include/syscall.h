/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _SYSCALL_H_
#define _SYSCALL_H_


#include <cdefs.h> /* for __DEAD */
#include <mips/trapframe.h>
struct trapframe; /* from <machine/trapframe.h> */
struct open_file{
	struct vnode* vnode;
	int mode;	// This is the way with which the file is open, like O_RDONLY, O_WRONLY, O_RDWR
	off_t starting_point;	//With this variable, it is possible for example reading a file not from the beginning but from a particular point
	struct lock* mylock;	// In this way open_file can be shared by concurrent threads
	//int flag_fileopen;	//When this flag is put equal to 1, it means that the file is open
};

/*
 * The system call dispatcher.
 */

void syscall(struct trapframe *tf);

/*
 * Support functions.
 */

/* Helper for fork(). You write this. */
void enter_forked_process(void *tf, unsigned long param);

/* Enter user mode. Does not return. */
__DEAD void enter_new_process(int argc, userptr_t argv, userptr_t env,
		       vaddr_t stackptr, vaddr_t entrypoint);


/*
 * Prototypes for IN-KERNEL entry points for system call implementations.
 */

int sys_reboot(int code);
int sys___time(userptr_t user_seconds, userptr_t user_nanoseconds);

// FILE SYSTEM CALLS
int sys_open (const char *filename, int flag, int *retfd);
int sys_read(int fd, userptr_t buf, size_t size, int *retval);
int sys_write(int fd, const userptr_t buf, size_t size, int *retval);
int sys_close(int fd);

//PROCESS SYSTEM CALLS
int sys_exit(int exitcode);
int sys_fork(struct trapframe *tf_parent, pid_t *child_pid);
int sys_getpid (pid_t *retval);
//int sys_execv(const char* program, char**args);
int sys_execv(const char* program, char**args1);
//int sys_execv(const char* program);
//int sys_execv(char**args);
int sys_waitpid (pid_t waiting_for_pid, int *status, int options, pid_t *retval);

#endif /* _SYSCALL_H_ */
