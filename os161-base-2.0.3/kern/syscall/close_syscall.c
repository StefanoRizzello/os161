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

int sys_close (int fd){

	//FIRST CHECKS
	if (fd < 0 || curproc->file_table[fd] == NULL){
	
		return EBADF;		//This error means: bad file number
	}
	
	//if (curproc->file_table[fd]->flag_fileopen == 1){
	
	lock_acquire (curproc->file_table[fd]->mylock);
	vfs_close(curproc->file_table[fd]->vnode);
	lock_release (curproc->file_table[fd]->mylock);
	lock_destroy (curproc->file_table[fd]->mylock);
	kfree(curproc->file_table[fd]);	
	curproc->file_table[fd] = NULL;
	return 0;
	
	//}
	
	/*else{
	
		return -1;
	
	}*/
}
