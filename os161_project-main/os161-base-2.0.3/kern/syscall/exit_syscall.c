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
#include <kern/wait.h>

int sys_exit(int exitcode){


	for(int i = 0; i < OPEN_MAX; i++){
		if (process_table[i] != NULL){
			if (curproc->process_id == process_table[i]->process_id){
				break;	//In this way if I find the process into the process table, I can exit immediately from the for cycle
			}
		}
		if (i == OPEN_MAX - 1){
			panic("The process is not into the process table");
		}
	}


	lock_acquire(curproc->mylock);
	//Critical section
	
	curproc->flag_exited = 1;
	curproc->exit_code = _MKWAIT_EXIT(exitcode*64);	
	//curproc->exit_code = _MKWAIT_EXIT(exitcode);
	
	cv_signal(curproc->mycv, curproc->mylock);
	lock_release(curproc->mylock);
	thread_exit();
	return 0;
}
