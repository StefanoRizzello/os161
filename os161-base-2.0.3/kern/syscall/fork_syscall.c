#include <types.h>
#include <stat.h>
#include <endian.h>
#include <lib.h>
#include <limits.h>
#include <copyinout.h>
#include <synch.h>
#include <vfs.h>
#include <proc.h>
#include <current.h>
#include <syscall.h>
#include <kern/unistd.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <vnode.h>
#include <uio.h>
#include <addrspace.h>
#include <thread.h>
#include <kern/fcntl.h>

int sys_fork(struct trapframe *tf_parent, pid_t *child_pid){

	struct proc *child_process;
	struct trapframe *tf_child;
	int i = 0;
	int error;
	
	child_process = create_myproc("child_process");
	if (child_process == NULL){
		return ENOMEM;	// Out of memory error
	}
	//Now I have to put the child process into the table that contains all the process
	while (process_table[i] != NULL){
		if (i == OPEN_MAX - 1){
			return ENPROC;		// Too many processes in system
		}
		i = i + 1;
	}
	process_table[i] = child_process;	// I put the child process into the table
	counter_ids = counter_ids + 1;	// In this way the id of each process will be different
	child_process->process_id = counter_ids;
	child_process->parent_id = curproc->process_id;
	*child_pid = child_process->process_id;	// In this way the return value of the fork for the parent process is the PID of the child process
	
	//Now I copy the file_table of the parent process
	for (i = 0; i < OPEN_MAX; i++){
		if (curproc->file_table[i] != NULL){
			child_process->file_table[i] = curproc->file_table[i];
		}
	}
	error = as_copy (curproc->p_addrspace, &child_process->p_addrspace);	// I have copied the address space of the parent process
	if (error != 0){
		return error;
	}
	
	//To call the thread_fork, it is need to have a copy of the trapframe of the parent
	
	tf_child = (struct trapframe *) kmalloc(sizeof(struct trapframe));
	if (tf_child == NULL){
		return ENOMEM;
	}
	*tf_child = *tf_parent;	//In this wat tf_child == tf_parent
	
	error = thread_fork("thread_child", child_process, enter_forked_process, tf_child, (unsigned long) NULL);
	if(error != 0){
		return error;
	}
	return 0;
}
