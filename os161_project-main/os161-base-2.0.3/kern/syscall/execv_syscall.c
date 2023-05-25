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

int sys_execv(const char* program, char**args){

	//I HAVE TOKEN THE CODE INTO runprogram.c AND THEN I HAVE MODIFIED IT 

	struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result;
	int error;
	//char progname [PATH_MAX];
	char *progname;
	size_t got;
	int i = 0;
	int j = 0;
	char kernel_buffer [OPEN_MAX];
	int actual_position = 0;
	int max_byte = ARG_MAX;	//Max bytes for an exec function
	int starting_values[OPEN_MAX];
	int addend = 0;
	int number_of_zeros = 0;
	int value = 0;
	userptr_t first_argv_offset;
		
	progname = kmalloc(PATH_MAX);
	
	error = copyinstr((const userptr_t) program, progname, PATH_MAX, &got);	//In this way I am copying the string program from user level address to kernel level address
	//error = copyin((const userptr_t)program, progname, PATH_MAX); 
	if (error != 0){
		return error;
	
	}
		
	while(args[i] != NULL){	//To indicate that the sequence of parameters is finished, we put at the end of args NULL
					//IMPORTANT: at first I want to create the buffer in which each argument is terminated by the correct number of '\0', then I want to copy it into the stack
		
		error = copyinstr((const userptr_t) args[i], & kernel_buffer[actual_position], max_byte, &got);
		if (error != 0){
			return error;
		}
		starting_values[i] = actual_position;
		actual_position = actual_position + got;	//In this way in the next cycle I write the new parameter starting from the first free position
		addend = got%4;	//In MIPS pointers must be aligned by 4
		if (addend == 0){
			max_byte = max_byte - got;	//In this way I decrease the maximum number of bytes that the arguments can have 
		}
		else{
			number_of_zeros = 4 - addend;
			for ( j = 0; j < number_of_zeros; j++){
				kernel_buffer[actual_position + j] = '\0';
			}
			max_byte = max_byte - got - number_of_zeros;	//In this way I decrease the maximum number of bytes that the arguments can have
			actual_position = actual_position + number_of_zeros;
		}
		i = i + 1;
	}
	
	/* Open the file. */

	result = vfs_open(progname, O_RDONLY, 0, &v);
	if (result != 0) {
		return result;
	}

	/* We should be a new process. */
	//KASSERT(proc_getas() == NULL);

	/* Create a new address space. */
	as = as_create();
	if (as == NULL) {
		vfs_close(v);
		return ENOMEM;
	}

	/* Switch to it and activate it. */
	proc_setas(as);
	as_activate();

	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		vfs_close(v);
		return result;
	}

	/* Done with the file now. */
	vfs_close(v);

	/* Define the user stack in the address space */
	result = as_define_stack(as, &stackptr);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		return result;
	}
	
	//Now I want to copy the arguments from the kernel_buffer to the user stack
	stackptr = stackptr - actual_position;
	error = copyout(kernel_buffer, (userptr_t) stackptr, actual_position);
	
	stackptr = stackptr - 4;
	char* last = NULL;
	error = copyout(&last, (userptr_t) stackptr, sizeof(last));
	if (error != 0){
		return error;
	}
	
	for (j  = i - 1; j >= 0; j--){
		stackptr = stackptr - 4;
		value = USERSTACK - actual_position + starting_values[j];
		last = (char*)value;
		error = copyout(&last, (userptr_t) stackptr, sizeof(last));
		if (error != 0){
			return error;
		} 
	}
	first_argv_offset = (userptr_t) stackptr;
	stackptr = stackptr - 4;
	
	/* Warp to user mode. */
	enter_new_process(i, first_argv_offset,
			  NULL /*userspace addr of environment*/,
			  stackptr, entrypoint);

	/* enter_new_process does not return. */
	panic("enter_new_process returned\n");
	return EINVAL;
}
