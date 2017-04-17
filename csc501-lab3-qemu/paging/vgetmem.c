/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */
WORD	*vgetmem(nbytes)
	unsigned nbytes;
{

//	kprintf("To be implemented!\n");
	STATWORD ps;
	int pid = currpid;
	struct mblock *p, *q, *leftover *memlist, *mptr;
	
	disable(ps);
	if(proctab[pid].vmemlist->mnext == (struct mblock*)NULL)
	{
		proctab[pid].vmemlist->mnext = (struct mblock*)(NBPG * proctab[pid].vhpno);
		proctab[pid].vmemlist->mlen = proctab[pid].vhpnpages * NBPG;
		
		mptr = proctab[pid].vmemlist->mnext;
		mptr->mnext = (struct mblock*)NULL;
		mptr->mlen = proctab[pid].vhpnpages * NBPG;
	}
	if(nbytes == 0)
	{
		restore(ps); return SYSERR;
	}
	nbytes = (unsigned int)roundmb(nbytes);
	memlist = proctab[pid].vmemlist;
	for(q=memlist, p = memlist->mnext; p != (struct mblock*)NULL; q=p, p = p->mnext)
	{
		if(p->mlen == nbytes)  // Exact size match
		{
			q->mnext = p->mnext;
			memlist->mlen -= nbytes;
			restore(ps);
			return ((WORD *)p);
		}
		else if(p->mlen > nbytes)
		{
			leftover = (struct mblock*)((unsigned)p + nbytes);
			q->mnext = leftover;
			leftover->mnext = p->mnext;
			leftover->mlen = p->mlen - nbytes;
			memlist->mlen -= nbytes;
			restore(ps);
			return ((WORD *)p);
		}
	}
	
	restore(ps);
	return((WORD*) SYSERR );
}
