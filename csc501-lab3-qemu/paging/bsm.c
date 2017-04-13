/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
	int i=0, j=0;
	STATWORD ps;
	disable(ps);
	//TODO: make changes according to the changes in the data structure
	for (i=0; i < BS_COUNT ; i++)
	{
		bsm_tab[i].bs_status = BSM_UNMAPPED;
		bsm_tab[i].bs_sem = -1;
		bsm_tab[i].shared = TRUE;
		bsm_tab[i].bs_npages = -1;
		for (j=0; j < MAX_PROCESS_PER_BS; j++)
		{
			bsm_tab[i].pr_map[j].bs_pid = -1;
			bsm_tab[i].pr_map[j].bs_vpno = -1;
		}
	}
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free (UNMAPPED) entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(bsd_t * bsm_id)
{
	/* Returns OK if there was one unmapped BS. bsm_id shall have that ID */
	STATWORD ps;
	int i;
	disable(ps);
	for(i=0; i< BS_COUNT; i++)
	{
		if (bsm_tab[i].bs_status == BSM_UNMAPPED)
		{
			bsm_id = i;
			return OK;
		}
	}
	restore(ps);
	return SYSERR;
}

int is_bsm_available(bsd_t bsm_id, int pid)
{
	/* Return TRUE if bsm_id is available for use by process ID pid*/
	if (bsm_tab[bsm_id].bs_status == BSM_UNMAPPED)
		return TRUE;
	if (bsm_tab[bsm_id].shared == TRUE)
		return TRUE;
	int i;
	for(i=0; i< MAX_PROCESS_PER_BS; i++)
	{
		if (bsm_tab[bsm_id].pr_map[i].bs_pid == pid)
			return TRUE;
	}
	return FALSE;
}
/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, unsigned long vaddr, int* store, int* pageth)
{
	int i, x;
	unsigned long vpno = vaddr >> 12;
	for(i=0; i<BS_COUNT; i++)
	{
		if(proctab[pid].bs_map[i].bs_status == BSM_MAPPED)
		{
			x = proctab[pid].bs_map[i].bs_vpno;
			if (vpno >= x && vpno < x + proctab[pid].bs_map[i].bs_npages)
			{
				*store = i;
				*pageth = vpno - x;
				return OK;
			}
		}
	}
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, bsd_t bs_id, int npages)
{
	if (bs_id < 0 || bs_id > BS_COUNT || pid < 1 || pid >= NPROC || npages < 1 
	|| npages > 256)
		return SYSERR;
	if (is_bsm_available(bs_id, currpid) == FALSE)
	{
		kprintf("\nBS # %d not available for process ID %d",bs_id, currpid);
		return SYSERR;
	}
	
	if(bsm_tab[bs_id].bs_status == BSM_MAPPED)
	{
		int i;	
		for (i=0; i< MAX_PROCESS_PER_BS; i++)
		{
			if(bsm_tab[bs_id].pr_map[i].bs_pid == -1)
			{
				bsm_tab[bs_id].pr_map[i].bs_pid = pid;
				bsm_tab[bs_id].pr_map[i].bs_vpno = vpno;
				
				proctab[pid].bs_map[bs_id].bs_status = BSM_MAPPED;
				proctab[pid].bs_map[bs_id].bs_vpno = vpno;
				proctab[pid].bs_map[bs_id].bs_npages = npages;
				proctab[pid].bs_map[bs_id].shared = TRUE;
				return OK;
			}
		}
		kprintf("\nBSM_MAP failed. Too many processes have already mapped to this BS id");
		return SYSERR;
	}
	else if(bsm_tab[bs_id].bs_status == BSM_UNMAPPED)
	{
		kprintf("\nFirst call get_bs ");
		return SYSERR;
	}
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete a mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
}


