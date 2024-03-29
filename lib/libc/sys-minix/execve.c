/* execve() - basic program execution call */

#include <sys/cdefs.h>
#include "namespace.h"
#include <lib.h>

#include <unistd.h>
#include <string.h>
#include <stddef.h>
#include <minix/param.h>
#include <sys/exec_elf.h>
#include <sys/exec.h>

int execve(const char *path, char * const *argv, char * const *envp)
{
	message m;
	size_t frame_size = 0;	/* Size of the new initial stack. */
	int argc = 0;		/* Argument count. */
	int envc = 0;		/* Environment count */
	char overflow = 0;	/* No overflow yet. */
	char *frame;
	struct ps_strings *psp;
	int vsp = 0;	/* (virtual) Stack pointer in new address space. */

	minix_stack_params(path, argv, envp, &frame_size, &overflow,
		&argc, &envc);

	/* The party is off if there is an overflow. */
	if (overflow) {
		errno = E2BIG;
		return -1;
	}

	/* Allocate space for the stack frame. */
	if ((frame = (char *) sbrk(frame_size)) == (char *) -1) {
		errno = E2BIG;
		return -1;
	}

	minix_stack_fill(path, argc, argv, envc, envp, frame_size, frame,
	       	&vsp, &psp);

	/* Clear unused message fields */
	memset(&m, 0, sizeof(m));

	/* We can finally make the system call. */
	m.PM_EXEC_NAME = (char *) __UNCONST(path);
	m.PM_EXEC_NAMELEN = strlen(path) + 1;
	m.PM_EXEC_FRAME = frame;
	m.PM_EXEC_FRAMELEN = frame_size;
	m.PM_EXEC_PS_STR = (char *)(vsp + ((char *)psp - frame));

	(void) _syscall(PM_PROC_NR, PM_EXEC, &m);

	/* Failure, return the memory used for the frame and exit. */
	(void) sbrk(-frame_size);

	return -1;
}
