/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
//	kprintf("To be implemented!\n");
	int pid = create(procaddr, ssize, priority, name, nargs, args);
	int status, bs_id;
	STATWORD ps;
	disable(ps);

	if (pid == SYSERR)
	{
		kprintf("\nProcess Creation failed"); 
		restore(ps); return SYSERR;
	}
	status = get_bsm(&bs_id);
	if(status == SYSERR)
	{
		kprintf("\nNo BS available BS for the virtual process");
		restore(ps); return SYSERR;
	}
	hsize = reserve_bs(pid, bs_id, hsize, FALSE);  // Get an exclusive BS
	if(hsize == SYSERR)
	{
		kprintf("\n get_bs failed even when get_bsm had succedded");
		restore(ps); return SYSERR;
	}
	status = bsm_map(pid, 4096, bs_id, hsize);
	if(status == SYSERR)
	{
		kprintf("\n bsm_map failed");
		restore(ps); return SYSERR;
	}
	
	proctab[pid].store = bs_id;
	proctab[pid].vhpno = 4096;
	proctab[pid].vhpnpages = hsize;
	// Now is the virtual heap allocation part.
	proctab[pid].vmemlist = (struct mblock*)getmem(sizeof(struct mblock));
	proctab[pid].vmemlist->mnext = (struct mblock*)(proctab[pid].vhpno * NBPG);
	proctab[pid].vmemlist->mlen = hsize * NBPG;

	struct mblock * mptr = proctab[pid].vmemlist->mnext;
	mptr->mnext = (struct mblock*)NULL;
	mptr->mlen = hsize * NBPG;
	// I hope i didn't miss anything. Let's do a Litmus test
	kprintf("\npid = %d \t vmemlist->mnext 0x%08x  \t vmemlist->mlen %d",pid, (unsigned)(proctab[pid].vmemlist->mnext), proctab[pid].vmemlist->mlen);
	kprintf("\n mptr 0x%08x  size = %d ",(unsigned)mptr, mptr->mlen);
	restore(ps);
	return OK;
}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
