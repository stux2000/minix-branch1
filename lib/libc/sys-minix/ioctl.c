#include <sys/cdefs.h>
#include "namespace.h"
#include <lib.h>
#include <stdarg.h>

#include <sys/ioctl.h>
#include <minix/i2c.h>
#include <string.h>
#include <sys/ioccom.h>
#include <stdarg.h>

static void rewrite_i2c_netbsd_to_minix(minix_i2c_ioctl_exec_t *out,
    i2c_ioctl_exec_t *in);
static void rewrite_i2c_minix_to_netbsd(i2c_ioctl_exec_t *out,
    minix_i2c_ioctl_exec_t *in);

static void rewrite_i2c_netbsd_to_minix(minix_i2c_ioctl_exec_t *out,
	i2c_ioctl_exec_t *in)
{
  memset(out, '\0', sizeof(minix_i2c_ioctl_exec_t));

  out->iie_op = in->iie_op;
  out->iie_addr = in->iie_addr;
  out->iie_cmdlen = I2C_EXEC_MAX_CMDLEN < in->iie_cmdlen ?
  	I2C_EXEC_MAX_CMDLEN : in->iie_cmdlen;
  out->iie_buflen = I2C_EXEC_MAX_BUFLEN < in->iie_buflen ?
  	I2C_EXEC_MAX_BUFLEN : in->iie_buflen;

  if (in->iie_cmdlen > 0 && in->iie_cmd != NULL) {
	memcpy(out->iie_cmd, in->iie_cmd, in->iie_cmdlen);
  }

  if (in->iie_buflen > 0 && in->iie_buf != NULL) {
	memcpy(out->iie_buf, in->iie_buf, in->iie_buflen);
  }
}

static void rewrite_i2c_minix_to_netbsd(i2c_ioctl_exec_t *out,
	minix_i2c_ioctl_exec_t *in)
{
  /* the only field that changes is iie_buf, everything else is the same */
  if (in->iie_buflen > 0 && in->iie_buf != NULL) {
	memcpy(out->iie_buf, in->iie_buf, in->iie_buflen);
  }
}

int     ioctl(int fd, unsigned long request, ...)
{
  int r, request_save;
  message m;
  void *addr;
  void *data;
  va_list ap;

  va_start(ap, request);
  data = va_arg(ap, void *);

  /*
   * To support compatibility with interfaces on other systems, certain
   * requests are re-written to flat structures (i.e. without pointers).
   */
  minix_i2c_ioctl_exec_t i2c;

  request_save = request;

  switch (request) {
	case I2C_IOCTL_EXEC:
		rewrite_i2c_netbsd_to_minix(&i2c, data);
		addr = (void *) &i2c;
		request = MINIX_I2C_IOCTL_EXEC;
		break;
	default:
		/* Keep original as-is */
		addr = (void *) data;
		break;
  }

  memset(&m, 0, sizeof(m));
  m.VFS_IOCTL_FD = fd;
  m.VFS_IOCTL_REQ = request;
  m.VFS_IOCTL_ARG = (char *) addr;

  r = _syscall(VFS_PROC_NR, VFS_IOCTL, &m);

  /* Translate back to original form */
  switch (request_save) {
	case I2C_IOCTL_EXEC:
		rewrite_i2c_minix_to_netbsd(data, &i2c);
		break;
	default:
		/* Nothing to do */
		break;
  }

  va_end(ap);

  return r;
}
