/* The kernel call implemented in this file:
 *   m_type:	SYS_SDEVIO
 *
 * The parameters for this kernel call are:
 *    m2_i3:	DIO_REQUEST	(request input or output)
 *    m2_l1:	DIO_PORT	(port to read/ write)
 *    m2_p1:	DIO_VEC_ADDR	(virtual address of buffer, or grant ID)
 *    m2_l2:	DIO_VEC_SIZE	(number of elements)
 *    m2_i2:	DIO_VEC_PROC	(process where buffer is)
 *    m2_i1:	DIO_OFFSET	(offset into the grant)
 */

#include "kernel/system.h"
#include <minix/devio.h>
#include <minix/endpoint.h>

#include "arch_proto.h"

#if USE_SDEVIO

/*===========================================================================*
 *			        do_sdevio                                    *
 *===========================================================================*/
int do_sdevio(struct proc * caller, message *m_ptr)
{
  vir_bytes newoffset;
  endpoint_t newep;
  int proc_nr;
  endpoint_t proc_nr_e = m_ptr->DIO_VEC_ENDPT;
  vir_bytes count = m_ptr->DIO_VEC_SIZE;
  long port = m_ptr->DIO_PORT;
  phys_bytes vir_buf;
  int i, req_type, req_dir, size, nr_io_range;
  struct priv *privp;
  struct io_range *iorp;
  struct proc *destproc;
  int retval;

  /* Allow safe copies and accesses to SELF */
  if ((m_ptr->DIO_REQUEST & _DIO_SAFEMASK) != _DIO_SAFE &&
	proc_nr_e != SELF)
  {
	static int first= 1;
	if (first)
	{
		first= 0;
		printf("do_sdevio: for %d, req %d\n",
			m_ptr->m_source, m_ptr->DIO_REQUEST);
	}
  }

  /* Check if process endpoint is OK. 
   * A driver may directly provide a pointer to a buffer at the user-process
   * that initiated the device I/O. Kernel processes, of course, are denied.
   */
  if (proc_nr_e == SELF)
	okendpt(caller->p_endpoint, &proc_nr);
  else
	if(!isokendpt(proc_nr_e, &proc_nr))
		return(EINVAL);
  if (iskerneln(proc_nr)) return(EPERM);

  /* Extract direction (in or out) and type (size). */
  req_dir = m_ptr->DIO_REQUEST & _DIO_DIRMASK;
  req_type = m_ptr->DIO_REQUEST & _DIO_TYPEMASK;

  /* Check for 'safe' variants. */
  if((m_ptr->DIO_REQUEST & _DIO_SAFEMASK) == _DIO_SAFE) {
     /* Map grant address to physical address. */
     if(verify_grant(proc_nr_e, caller->p_endpoint,
	(cp_grant_id_t) m_ptr->DIO_VEC_ADDR,
	count,
	req_dir == _DIO_INPUT ? CPF_WRITE : CPF_READ,
	(vir_bytes) m_ptr->DIO_OFFSET, 
	&newoffset, &newep, NULL) != OK) {
	printf("do_sdevio: verify_grant failed\n");
	return EPERM;
    }
	if(!isokendpt(newep, &proc_nr))
		return(EINVAL);
     destproc = proc_addr(proc_nr);
     vir_buf = newoffset;
  } else {
     if(proc_nr != _ENDPOINT_P(caller->p_endpoint))
     {
	printf("do_sdevio: unsafe sdevio by %d in %d denied\n",
		caller->p_endpoint, proc_nr_e);
	return EPERM;
     }
     /* Get and check physical address. */
     vir_buf = (phys_bytes) m_ptr->DIO_VEC_ADDR;
     destproc = proc_addr(proc_nr);
  }
     /* current process must be target for phys_* to be OK */

  switch_address_space(destproc);

  switch (req_type)
  {
	case _DIO_BYTE: size= 1; break;
	case _DIO_WORD: size= 2; break;
	case _DIO_LONG: size= 4; break;
	default: size= 4; break;	/* Be conservative */
  }

  privp= priv(caller);
  if (privp && privp->s_flags & CHECK_IO_PORT)
  {
	port= m_ptr->DIO_PORT;
	nr_io_range= privp->s_nr_io_range;
	for (i= 0, iorp= privp->s_io_tab; i<nr_io_range; i++, iorp++)
	{
		if (port >= iorp->ior_base && port+size-1 <= iorp->ior_limit)
			break;
	}
	if (i >= nr_io_range)
	{
		printf(
		"do_sdevio: I/O port check failed for proc %d, port 0x%x\n",
			m_ptr->m_source, port);
		retval = EPERM;
		goto return_error;
	}
  }

  if (port & (size-1))
  {
	printf("do_devio: unaligned port 0x%x (size %d)\n", port, size);
	retval = EPERM;
	goto return_error;
  }

  /* Perform device I/O for bytes and words. Longs are not supported. */
  if (req_dir == _DIO_INPUT) { 
      switch (req_type) {
      case _DIO_BYTE: phys_insb(port, vir_buf, count); break; 
      case _DIO_WORD: phys_insw(port, vir_buf, count); break; 
      default:
  		retval = EINVAL;
		goto return_error;
      } 
  } else if (req_dir == _DIO_OUTPUT) { 
      switch (req_type) {
      case _DIO_BYTE: phys_outsb(port, vir_buf, count); break; 
      case _DIO_WORD: phys_outsw(port, vir_buf, count); break; 
      default:
  		retval = EINVAL;
		goto return_error;
      } 
  }
  else {
	  retval = EINVAL;
	  goto return_error;
  }
  retval = OK;

return_error:
  /* switch back to the address of the process which made the call */
  switch_address_space(caller);
  return retval;
}

#endif /* USE_SDEVIO */
