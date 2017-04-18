#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {

	/* requests a new mapping of npages with ID map_id for the process calling this of shared as TRUE*/
	return reserve_bs(currpid, bs_id, npages, TRUE);
}

int reserve_bs(int pid, bsd_t bs_id, unsigned int npages, int shared)
{
	int bs_shared;
	STATWORD ps;
	disable(ps);
    if (bs_id < 0 || bs_id > BS_COUNT || npages <= 0 || npages > 256 || pid <0 || pid >= NPROC)
	{
		kprintf("\nInvalid inputs to reserve_bs. pid = %d, bs_id= %d, npages= %d", pid, bs_id, npages);
		restore(ps);
		return SYSERR;
	}
	
	if (is_bsm_available(bs_id, pid, &bs_shared) == FALSE)
	{
		kprintf("\nBS # %d not available for process ID %d",bs_id, pid);
		restore(ps);
		return SYSERR;
	}

	int i=0;
	if(bs_shared == TRUE)
	{// If pid already had an entry then, simply update the entry.
		for (i=0; i< MAX_PROCESS_PER_BS; i++)
		{
			if(bsm_tab[bs_id].pr_map[i].bs_pid == pid)
				break;
		}
		if(i == MAX_PROCESS_PER_BS)
		{	// Not found, Now look for available slot 
			for (i=0; i< MAX_PROCESS_PER_BS; i++)
			{
				if(bsm_tab[bs_id].pr_map[i].bs_pid == -1)
				{bsm_tab[bs_id].pr_map[i].bs_pid = pid; break;}
			}
		}
	}
	else
	{
		bsm_tab[bs_id].pr_map[0].bs_pid = pid;
	}
	if (bsm_tab[bs_id].bs_status == BSM_UNMAPPED)
		bsm_tab[bs_id].bs_npages = npages;
	bsm_tab[bs_id].bs_status = BSM_MAPPED;
	bsm_tab[bs_id].shared = shared;
	

	if(i== MAX_PROCESS_PER_BS)
	{
		kprintf("\nGET_BS failed. MAX_PROCESS_PER_BS have already reserved this Backing store");
		restore(ps);
		return SYSERR;
	}
	
	proctab[pid].bs_map[bs_id].bs_status = BSM_MAPPED;
	proctab[pid].bs_map[bs_id].bs_npages = bsm_tab[bs_id].bs_npages;
	proctab[pid].bs_map[bs_id].shared = shared;
	restore(ps);
	return bsm_tab[bs_id].bs_npages;

}
