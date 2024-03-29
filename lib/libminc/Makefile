#
# Minimal libc for servers and drivers.
#

# LSC: TODO: Explaination of how this works
#.for f in \
#${f}:	${NETBSDSRCDIR}//${f:C/\.o/.S/}
#OBJS+=	${f}
#CLEANFILES+= ${f}
#.endfor

.include <bsd.own.mk>

LIB=		minc

MKPIC=		no # Never used as a dynamic library

LIBSADIR=	${NETBSDSRCDIR}/sys/lib/libsa
LIBSYSDIR=	${NETBSDSRCDIR}/lib/libsys
LIBCDIR=	${NETBSDSRCDIR}/lib/libc
LIBCARCHDIR=	${LIBCDIR}/arch/${MACHINE_CPU}
LIBCOMMONCDIR=	${NETBSDSRCDIR}/common/lib/libc
LIBCOMMONCARCHDIR= ${LIBCOMMONCDIR}/arch/${MACHINE_CPU}

CPPFLAGS+=	-I${LIBCDIR}/include -I${LIBCDIR}
CPPFLAGS+=	-D_LIBMINC

CFLAGS+=	-fno-builtin

#
# Customized versions of libc functions.
#
SRCS+= atoi.c fputs.c _snprintf.c strtol.c
CPPFLAGS._snprintf.c+=	-I${LIBSADIR}

CPPFLAGS.strtol.c+=	-D_STANDALONE
CPPFLAGS.strtol.c+=	-I${LIBCOMMONCDIR}/stdlib
CPPFLAGS.strtol.c+=	-I${NETBSDSRCDIR}/sys

########################################################################
#
# Functions imported from libsa (StandAlone)
#
.for f in \
	errno.o printf.o sprintf.o strerror.o subr_prf.o
${f}:	${LIBSADIR}/${f:C/\.o/.c/}
OBJS+=	${f}
CLEANFILES+= ${f}
.endfor

# LSC: I would like not to have to copy te file, but I can't
#      have libsa appear first in the .PATH, for a lot of files
#      would be taken from there, which is not the intention ATM.
CPPFLAGS.strerror.c+= -I${LIBSADIR}

# LSC: putchar and kputc have the same role / signature.
CPPFLAGS.subr_prf.c+= -Dputchar=kputc

# Activate optional support, may be deactivated.
CPPFLAGS.subr_prf.c+= -DLIBSA_PRINTF_LONGLONG_SUPPORT
CPPFLAGS.subr_prf.c+= -DLIBSA_PRINTF_WIDTH_SUPPORT

########################################################################
#
# Imports from libsys
#
.for f in \
	kputc.o sys_diagctl.o
${f}:	${LIBSYSDIR}/${f:C/\.o/.c/}
OBJS+=	${f}
CLEANFILES+= ${f}
.endfor

########################################################################
#
# Shared libc with userspace (/common/lib/libc)
#

.for f in \
	bswap64.o rb.o
${f}:	${LIBCOMMONCDIR}/gen/${f:C/\.o/.c/}
OBJS+=	${f}
CLEANFILES+= ${f}
.endfor

CPPFLAGS.bswap64.c+=	-D_STANDALONE
CPPFLAGS.rb.c+=		-D_LIBC

.for f in \
	sha2.o
${f}:	${LIBCOMMONCDIR}/hash/sha2/${f:C/\.o/.c/}
OBJS+=	${f}
CLEANFILES+= ${f}
.endfor

CPPFLAGS.sha2.c+=	-D_STANDALONE
CPPFLAGS.sha2.c+=	-I${NETBSDSRCDIR}/sys

.for f in \
	divdi3.o moddi3.o udivdi3.o umoddi3.o qdivrem.o
${f}:	${LIBCOMMONCDIR}/quad/${f:C/\.o/.c/}
OBJS+=	${f}
CLEANFILES+= ${f}
.endfor

.for f in \
	random.o strtoul.o
${f}:	${LIBCOMMONCDIR}/stdlib/${f:C/\.o/.c/}
OBJS+=	${f}
CLEANFILES+= ${f}
.endfor

CPPFLAGS.strtoul.c+=	-D_STANDALONE
CPPFLAGS.strtoul.c+=	-I${NETBSDSRCDIR}/sys

.for f in \
	strcasecmp.o strncasecmp.o strnlen.o strlcat.o
${f}:	${LIBCOMMONCDIR}/string/${f:C/\.o/.c/}
OBJS+=	${f}
CLEANFILES+= ${f}
.endfor

########################################################################
#
# Functions imported directly from libc.
#

.include "${.CURDIR}/arch/${MACHINE_ARCH}/Makefile.libc.inc"

# This file is specifically generated
SRCS+= errlist.c

errlist.c: ${LIBCDIR}/gen/errlist.awk \
	   ${NETBSDSRCDIR}/sys/sys/errno.h
	${TOOL_CAT} ${NETBSDSRCDIR}/sys/sys/errno.h | ${TOOL_SED} 's/(_SIGN//' | ${TOOL_AWK} -f ${LIBCDIR}/gen/errlist.awk > ${.TARGET}
CLEANFILES+=	errlist.c

.for f in \
	_errno.o \
	getprogname.o setprogname.o execle.o sleep.o time.o \
	ctype_.o tolower_.o toupper_.o usleep.o sigsetops.o
${f}:	${LIBCDIR}/gen/${f:C/\.o/.c/}
OBJS+=	${f}
CLEANFILES+= ${f}
.endfor

CPPFLAGS.ctype_.c+= -I${LIBCDIR}/locale
CPPFLAGS.isctype.c+= -I${LIBCDIR}/locale
CPPFLAGS.tolower_.c+= -I${LIBCDIR}/locale
CPPFLAGS.toupper_.c+= -I${LIBCDIR}/locale

.for f in \
	waitpid.o
${f}:	${LIBCDIR}/gen/minix/${f:C/\.o/.c/}
OBJS+=	${f}
CLEANFILES+= ${f}
.endfor

.for f in \
	initfini.o stack_protector.o
${f}:	${LIBCDIR}/misc/${f:C/\.o/.c/}
OBJS+=	${f}
CLEANFILES+= ${f}
.endfor

CPPFLAGS.stack_protector.c+= -Dxprintf=printf

.for f in \
	regcomp.o regerror.o regexec.o regfree.o
${f}:	${LIBCDIR}/regex/${f:C/\.o/.c/}
OBJS+=	${f}
CLEANFILES+= ${f}
.endfor

.for f in \
	abort.o atexit.o _env.o exit.o getenv.o \
	ldiv.o malloc.o \
	_rand48.o lrand48.o srand48.o
${f}:	${LIBCDIR}/stdlib/${f:C/\.o/.c/}
OBJS+=	${f}
CLEANFILES+= ${f}
# LSC FIXME: Try to get this out of the loop
CPPFLAGS.${i}+=	-I${LIBCDIR}/stdlib
.endfor

.for f in \
	strcspn.o
${f}:	${LIBCDIR}/string/${f:C/\.o/.c/}
OBJS+=	${f}
CLEANFILES+= ${f}
.endfor
CPPFLAGS.strcspn.c+=	-D_LIBC

.for f in \
	access.o brk.o close.o environ.o execve.o fork.o fsync.o \
	getgid.o getpid.o geteuid.o getuid.o gettimeofday.o getvfsstat.o \
	init.o link.o loadname.o lseek.o _mcontext.o mknod.o \
	mmap.o nanosleep.o open.o pread.o pwrite.o read.o sbrk.o \
	select.o setuid.o sigprocmask.o stack_utils.o stat.o stime.o \
	syscall.o _ucontext.o umask.o unlink.o write.o \
	kill.o
${f}:	${LIBCDIR}/sys-minix/${f:C/\.o/.c/}
OBJS+=	${f}
CLEANFILES+= ${f}
.endfor

CPPFLAGS.malloc.c+= -D_LIBSYS

.for f in \
	brksize.o _do_kernel_call_intr.o get_minix_kerninfo.o _ipc.o ucontext.o
${f}:	${LIBCARCHDIR}/sys-minix/${f:C/\.o/.S/}
OBJS+=	${f}
CLEANFILES+= ${f}
.endfor

.include <bsd.lib.mk>
