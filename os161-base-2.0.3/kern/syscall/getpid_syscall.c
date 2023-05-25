#include <types.h>
#include <stat.h>
#include <endian.h>
#include <lib.h>
#include <proc.h>
#include <current.h>
#include <syscall.h>

int sys_getpid (pid_t *retval){

	*retval = curproc->process_id;
	return 0;
}
