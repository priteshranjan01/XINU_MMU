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
	STATWORD ps;
	int status;
	disable(ps);
	status =  bsm_map(currpid, virtpage, source, npages);
	if(status != OK)
		kprintf("\nxmmap failed because bsm_map returned SYSERR");
	restore(ps);
	return status;
	}


/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{
	STATWORD ps;
	int status;
	disable(ps);
	status = bsm_unmap(currpid, virtpage, 0);
	if(status != OK)
		kprintf("\nxmunmap failed because bsm_unmap returned SYSERR");
	restore(ps);
	return status;
}
