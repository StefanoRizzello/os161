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

int sys_read(int fd, userptr_t buf, size_t size, int *retval){
	struct iovec iov;
	struct uio u;
	void *my_buf;
	int error;
	off_t amount_read;

	//FIRST CHECKS
	if (fd < 0 || curproc->file_table[fd]->mode == O_WRONLY || curproc->file_table[fd] == NULL){
	
		return EBADF;		//This error means: bad file number
	}
	
	my_buf = (void*) kmalloc (sizeof(buf)*size);
	lock_acquire (curproc->file_table[fd]->mylock);	//In this way the operation of reading will be made only by a thread
	uio_kinit(&iov, &u, my_buf, size, curproc->file_table[fd]->starting_point, UIO_READ);	//We initialize the structure, and in particular my_buf will be pointed by this structure
	error = VOP_READ(curproc->file_table[fd]->vnode, &u);	//Here there is the real operation of reading, and the structure u will be updated, and so my_buf will have the content of the reading
															//and also the content of the offset will be updated.
	if (error != 0){
		lock_release (curproc->file_table[fd]->mylock);
		kfree(my_buf);	// We delete the buffer
		return error;
	}
	amount_read = u.uio_offset - curproc->file_table[fd]->starting_point;
	curproc->file_table[fd]->starting_point = u.uio_offset;		//I update the starting point of the file into the file_table
	*retval = (int)amount_read;
	if (amount_read != 0){
	
		error = copyout (my_buf, buf, (size_t)amount_read);	//Copy a block of memory of length LEN from kernel address SRC to user-level address USERDEST. 
									//WE READ A NUMBER OF BYTES
		if (error != 0){
			lock_release (curproc->file_table[fd]->mylock);
			kfree(my_buf);	// We delete the buffer
			return error;
		}		
	}
	kfree(my_buf);
	lock_release (curproc->file_table[fd]->mylock);
	return 0;
}
