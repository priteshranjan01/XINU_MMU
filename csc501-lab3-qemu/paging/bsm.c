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
	bsd_t i;
	disable(ps);
	for(i=0; i< BS_COUNT; i++)
	{
		if (bsm_tab[i].bs_status == BSM_UNMAPPED)
		{
			*bsm_id = i;
			return OK;
		}
	}
	restore(ps);
	return SYSERR;
}

int is_bsm_available(bsd_t bsm_id, int pid, int *shared)
{
	/* Return TRUE if bsm_id is available for use by process ID pid*/
	//NOTE: Disable interrupts before calling this method.
	int i;
	if (bsm_tab[bsm_id].bs_status == BSM_UNMAPPED)
	{
		*shared = TRUE;
		return TRUE;
	}
	
	if (bsm_tab[bsm_id].shared == TRUE)
	{
		*shared = TRUE;
		for(i=0; i< MAX_PROCESS_PER_BS; i++)
		{
			if (bsm_tab[bsm_id].pr_map[i].bs_pid == pid || bsm_tab[bsm_id].pr_map[i].bs_pid == -1)
				return TRUE;
		}
	}
	else
	{
		*shared = FALSE;
		for(i=0; i< MAX_PROCESS_PER_BS; i++)
		{
			if (bsm_tab[bsm_id].pr_map[i].bs_pid == pid)
				return TRUE;
		}
	}
	return FALSE;
}
/*-------------------------------------------------------------------------
 * free_bsm - free all entries from bsm_tab of process pid
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int pid)
{
	int i;
	for(i = 0; i < BS_COUNT; i++)
	{
		if(proctab[pid].bs_map[i].bs_status == BSM_MAPPED)
		{
			__release_bs__(pid, i);
		}
	}
	return OK;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, unsigned long vaddr, int* store, int* pageth)
{
	int i;
	// NOTE: Disable interrupts before calling this method.
	unsigned long vpno = vaddr >> 12;
	unsigned long x=0;
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
	kprintf("\nBSM lookup failed");
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, bsd_t bs_id, int npages)
{
	/*
	This method maps a process' virtual address space of size npages starting 
	at (vpno * 4096) to backing store number bs_id.
	
	On success, this inserts the pid into bsm_tab stucture and proctab.
	MAX_PROCESS_PER_BS controls the number of processes that can have 
		concurrent mapping to a backing store.
	Make sure that status of bs_id is BSM_MAPPED before calling this function.
	NOTE: Disable interrupts before calling this method.
	*/
	if (bs_id < 0 || bs_id > BS_COUNT || pid < 1 || pid >= NPROC || npages < 1 || npages > 256)
		return SYSERR;
	
	int bs_shared;
	if (is_bsm_available(bs_id, pid, &bs_shared) == FALSE)
	{
		kprintf("\nBS # %d not available for process ID %d",bs_id, pid);
		return SYSERR;
	}
	
	if(bsm_tab[bs_id].bs_status == BSM_MAPPED)
	{
		int i;	
		// If pid already had an entry then, simply update the entry.
		for (i=0; i< MAX_PROCESS_PER_BS; i++)
		{
			if(bsm_tab[bs_id].pr_map[i].bs_pid == pid)
			{
				bsm_tab[bs_id].pr_map[i].bs_pid = pid;
				bsm_tab[bs_id].pr_map[i].bs_vpno = vpno;
				
				proctab[pid].bs_map[bs_id].bs_status = BSM_MAPPED;
				proctab[pid].bs_map[bs_id].bs_vpno = vpno;
				//proctab[pid].bs_map[bs_id].bs_npages = npages;
				proctab[pid].bs_map[bs_id].shared = bs_shared;
				return OK;
			}
		}
		// Now look for available slot 
		for (i=0; i< MAX_PROCESS_PER_BS; i++)
		{
			if(bsm_tab[bs_id].pr_map[i].bs_pid == -1)
			{
				bsm_tab[bs_id].pr_map[i].bs_pid = pid;
				bsm_tab[bs_id].pr_map[i].bs_vpno = vpno;
				
				proctab[pid].bs_map[bs_id].bs_status = BSM_MAPPED;
				proctab[pid].bs_map[bs_id].bs_vpno = vpno;
				//proctab[pid].bs_map[bs_id].bs_npages = npages;
				proctab[pid].bs_map[bs_id].shared = bs_shared;
				return OK;
			}
		}
		kprintf("\nBSM_MAP failed. MAX_PROCESS_PER_BS have already mapped to this Backing store");
		return SYSERR;
	}
	kprintf("\nFirst call get_bs to gain rights on BS # %d", bs_id);
	return SYSERR;
}

/*-------------------------------------------------------------------------
 * bsm_unmap - delete a mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
	/*
	This method call only removes the vpno information from the mapping. 
	NOTE: Disable interrupts before calling this method.
	*/
	int store, pageth, i;
	if(bsm_lookup(pid, vpno<<12, &store, &pageth) == OK)
	{
		proctab[pid].bs_map[store].bs_vpno = -1;
		proctab[pid].bs_map[store].bs_npages = -1;
		proctab[pid].bs_map[store].shared = TRUE;
		
		for(i=0; i < MAX_PROCESS_PER_BS; i++)
		{
			if(bsm_tab[store].pr_map[i].bs_pid == pid)
			{
				//bsm_tab[store].pr_map[i].bs_pid = -1;
				bsm_tab[store].pr_map[i].bs_vpno = -1;
				return OK;
			}
		}
	}
	kprintf("\nbsm_lookup failed while doing a bsm_unmap ");
	return SYSERR;
}

void print_backing_store()
{
	int i, j;
	kprintf("\nBacking store map table\nStatus  shared npages pid vpno");
	for (i=0; i < BS_COUNT ; i++)
	{
		kprintf("\n%d\t %d\t %d\t",bsm_tab[i].bs_status, bsm_tab[i].shared, bsm_tab[i].bs_npages);
		for (j=0; j < MAX_PROCESS_PER_BS; j++)
		{
			if(bsm_tab[i].pr_map[j].bs_pid != -1)
			{
				kprintf("\n\t\t\t %d\t %d", bsm_tab[i].pr_map[j].bs_pid, bsm_tab[i].pr_map[j].bs_vpno);
			}
		}
	}
}
