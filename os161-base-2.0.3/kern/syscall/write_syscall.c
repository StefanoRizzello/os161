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

int sys_write(int fd, const userptr_t buf, size_t size, int *retval){
	struct iovec iov;
	struct uio u;
	void *my_buf;
	int error;
	off_t amount_write;
	int return_copyin;

	//FIRST CHECKS
	if (fd < 0 || curproc->file_table[fd]->mode == O_RDONLY || curproc->file_table[fd] == NULL){
	//if (curproc->file_table[fd]->mode == O_RDONLY){
	
		return EBADF;		//This error means: bad file number
	}
	
	//It is necessary to translate the message from the user-level address to the kernel-level address, and this is made by copyin
		
	my_buf = (void*) kmalloc (sizeof(buf)*size);
	return_copyin = copyin(buf, my_buf, size);	//Copy a block of memory of length LEN from user-level address USERSRC to kernel address DEST
							//WE WRITE A NUMBER OF BYTES
	if (return_copyin != 0){
		kfree(my_buf);	// We delete the buffer
		return return_copyin;
	}
	
	lock_acquire (curproc->file_table[fd]->mylock);	//In this way the operation of reading will be made only by a thread
	uio_kinit(&iov, &u, my_buf, size, curproc->file_table[fd]->starting_point, UIO_WRITE);	//We initialize the structure, and in particular my_buf will be pointed by this structure
	error = VOP_WRITE(curproc->file_table[fd]->vnode, &u);	
	
	if (error != 0){
		lock_release (curproc->file_table[fd]->mylock);
		kfree(my_buf);	// We delete the buffer
		return error;
	}

	amount_write = u.uio_offset - curproc->file_table[fd]->starting_point;
	curproc->file_table[fd]->starting_point = u.uio_offset;		//I update the starting point of the file into the file_table
	*retval = (int)amount_write;

	kfree(my_buf);
	lock_release (curproc->file_table[fd]->mylock);
	return 0;
}