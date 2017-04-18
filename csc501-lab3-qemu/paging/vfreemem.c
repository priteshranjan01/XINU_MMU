/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vmemlist
 *------------------------------------------------------------------------
 */
SYSCALL	vfreemem(block, size)
	struct	mblock	*block;
	unsigned size;
{
	//kprintf("To be implemented!\n");
	STATWORD ps;
	struct mblock *p, *q, *memlist;
	unsigned top;
	int pid = currpid;
	
	if(size == 0 || proctab[pid].vmemlist->mnext == (struct mblock*)NULL)
	{
		if(debug) kprintf("\n size = %d , vmemlist = 0x%08x ",size, proctab[pid].vmemlist->mnext);
		return SYSERR;
	}
	size = (unsigned)roundmb(size);
	disable(ps);
	memlist = proctab[pid].vmemlist;
	for(q = memlist, p = memlist->mnext; p!= (struct mblock*)NULL && p < block; q = p, p = p->mnext)
		;
	top = q->mlen + (unsigned)q ;
	// Check if block doesn't lie in free memory region
	if((top > (unsigned)block && q != memlist) || (p!= NULL && (size+(unsigned)block) > (unsigned)p))
	{
		restore(ps);
		return SYSERR;
	}
	// If block is adjacent to q, then coalesce with q
	if(q!= memlist && top == (unsigned)block)
	{
		q->mlen += size;
		memlist->mlen += size;
	}
	else
	{
		block->mlen = size;
		block->mnext = p;
		q->mnext = block;
		q = block;
		memlist->mlen += size;
	}
	// Check if it was also adjacent to the next available memory block
	if((unsigned)(q->mlen + (unsigned)q) == (unsigned)p)
	{
		q->mlen += p->mlen;
		q->mnext = p->mnext;
	}
	restore(ps);
	return(OK);
}
