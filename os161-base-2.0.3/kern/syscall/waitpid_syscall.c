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

int sys_waitpid (pid_t waiting_for_pid, int *status, int options, pid_t *retval){

	int index;
	int error;

	//First control on the waiting_for_pid
	if (waiting_for_pid == curproc->process_id){
		return ECHILD;	//No child processes
	}
	if (options != 0){
		return EINVAL;	//Invalid argument   I have never used a value of options different than 0
	}
	//Now I have to verify that the child process exists, and it is into the process table
	for (index = 0; index < OPEN_MAX; index ++){
		if (process_table[index] != NULL){
			if (waiting_for_pid == process_table[index]->process_id){		
				break;	//In this way I go out from the for cycle and I can continue the function with the correct value of index 
			}
			if (index == OPEN_MAX - 1){
				return ESRCH;	//No such process
			}
		}
	}
	//I want to be sure that the waiting_for_pid is the child of the curproc
	if (process_table[index]->process_id != 2){	//This modification is made for the menù
		if (curproc->process_id != process_table[index]->parent_id){
			return ECHILD;	//No child processes
		}
	}

	lock_acquire(process_table[index]->mylock);
	if (process_table[index]->flag_exited == 1){
		if (status != NULL){
			//I want to move the exit_code into the status in waitpid
			error = copyout(&process_table[index]->exit_code, (userptr_t) status, sizeof (process_table[index]->exit_code));
			if (error != 0){
				lock_release(process_table[index]->mylock);
				proc_destroy(process_table[index]);	//First I destroy the process and then I put the pointer into the process_table equal to NULL
				process_table[index] = NULL;
				return error;
			}
		}
		
		lock_release(process_table[index]->mylock);
		*retval = process_table[index]->process_id;
		proc_destroy(process_table[index]);	//First I destroy the process and then I put the pointer into the process_table equal to NULL
		process_table[index] = NULL;
		
		return 0;
	}
	
		cv_wait(process_table[index]->mycv, process_table[index]->mylock);
	
		if (status != NULL){
		
			//I want to move the exit_code into the status in waitpid
			error = copyout(&process_table[index]->exit_code, (userptr_t) status, sizeof (process_table[index]->exit_code));
			if (error != 0){
				lock_release(process_table[index]->mylock);
				proc_destroy(process_table[index]);	//First I destroy the process and then I put the pointer into the process_table equal to NULL
				process_table[index] = NULL;
				
				return error;
			}
		}
		
		lock_release(process_table[index]->mylock);
		*retval = process_table[index]->process_id;
		proc_destroy(process_table[index]);	//First I destroy the process and then I put the pointer into the process_table equal to NULL
		process_table[index] = NULL;
		
		return 0;
}
