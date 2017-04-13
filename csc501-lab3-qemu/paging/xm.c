/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
  //kprintf("xmmap - to be implemented!\n");
  int status = bsm_map(currpid, virtpage, source, npages);
  return status;
}


/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{
  kprintf("To be implemented!");
  return SYSERR;
}
