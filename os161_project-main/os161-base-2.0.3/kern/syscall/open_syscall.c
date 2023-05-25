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

int a = 0;

int sys_open (const char *filename, int flag, int *retfd){
	size_t actual;
	int i = 3;
	char kern_filename [PATH_MAX];
	int error = 0;	// error = 0 means that there is no error
	struct stat info_file;	//This struct is defined in stat.h
	int how;
	
	// FIRST CHECKS
	//I want to control that the process has a number of open files lower than OPEN_MAX, and I start from i = 3 because: 
	// 0 -> stdin
	// 1 -> stdout
	// 2 -> stderr
	
	while (curproc->file_table[i] != NULL){
		if (i == OPEN_MAX - 1){
		
			return EMFILE;		// EMFILE is an error. It means that a process has too many files opened
		}
	
		i = i + 1;
	}
	
	int copyk_filename = copyinstr ((const_userptr_t) filename, kern_filename, PATH_MAX, &actual);	// I am copying a string from user level address to kernel level address
	if (copyk_filename != 0){
	
		return copyk_filename;	// It means that copyinstr worked unsuccessfully
	
	}
	
	curproc->file_table[i] =  (struct open_file*) kmalloc(sizeof(struct open_file)); // In this way I put a connection between the file_table and the system file table
	KASSERT(curproc->file_table[i] != NULL);	//A check of the previous function
	error = vfs_open (kern_filename, flag, 0, &curproc->file_table[i]->vnode);	// the last argument has & because the function vfs_open wants as type struct vnode**
	if (error != 0){
		kfree(curproc->file_table[i]);	// In this way I increase the free memory
		curproc->file_table[i] = NULL;
		return error;
	}
		
	how = flag & O_ACCMODE;		// I took this part of code from the vfs_open, in vfspath.c file

	switch (how) {
	    case O_RDONLY:
	    
		curproc->file_table[i]->mode = O_RDONLY;
		
		break;
	    case O_WRONLY:
	    
	    	curproc->file_table[i]->mode = O_WRONLY;
	    	
	    	break;
	    case O_RDWR:
	    
		curproc->file_table[i]->mode = O_RDWR;
		
		break;
	    default:
	    	//In theory we shouldn't never be in this part of code, because this control is made in vfs_open. But as protection we implement it
	    	vfs_close(curproc->file_table[i]->vnode);
	    	kfree(curproc->file_table[i]);	// In this way I increase the free memory
			curproc->file_table[i] = NULL; 
			return EINVAL;
	}
	
	if (flag & O_APPEND){
		
		error = VOP_STAT (curproc->file_table[i]->vnode, &info_file);
		if (error != 0){
			vfs_close(curproc->file_table[i]->vnode);
			kfree(curproc->file_table[i]);	// In this way I increase the free memory
			curproc->file_table[i] = NULL;
			return error;
		}
		else{
		
			curproc->file_table[i]->starting_point = info_file.st_size;
		}
	
	}
	else{
		curproc->file_table[i]->starting_point = 0;
	
	}

	curproc->file_table[i]->mylock = lock_create("Lock creation");
	if (curproc->file_table[i]->mylock == NULL){
		vfs_close(curproc->file_table[i]->vnode);
		kfree(curproc->file_table[i]);	// In this way I increase the free memory
		curproc->file_table[i] = NULL;
	}

	//curproc->file_table[i]->flag_fileopen = 1;
	*retfd = i;	//The i-esimo open file
	return 0;
}
