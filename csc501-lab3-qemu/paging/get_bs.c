#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {

	/* requests a new mapping of npages with ID map_id */
	/*
	
	*/
	int bs_shared;
	STATWORD ps;
	disable(ps);
    if (bs_id < 0 || bs_id > BS_COUNT || npages <= 0 || npages > 256)
	{
		kprintf("\nInvalid inputs to get_bs procedure");
		restore(ps);
		return SYSERR;
	}
	
	if (is_bsm_available(bs_id, currpid, &bs_shared) == FALSE)
	{
		kprintf("\nBS # %d not available for process ID %d",bs_id, currpid);
		restore(ps);
		return SYSERR;
	}
	
	if (bsm_tab[bs_id].bs_status == BSM_UNMAPPED)
		bsm_tab[bs_id].bs_npages = npages;
	bsm_tab[bs_id].bs_status = BSM_MAPPED;
	bsm_tab[bs_id].shared = bs_shared;
	
	int i;
	for (i=0; (i< MAX_PROCESS_PER_BS) && (bs_shared == TRUE); i++)
	{
		if(bsm_tab[bs_id].pr_map[i].bs_pid == -1 || bsm_tab[bs_id].pr_map[i].bs_pid == currpid)
		{
			bsm_tab[bs_id].pr_map[i].bs_pid = currpid;  // vpno shall be set in bsm_map
			break;
		}
	}
	if(i== MAX_PROCESS_PER_BS)
	{
		kprintf("\nGET_BS failed. MAX_PROCESS_PER_BS have already reserved this Backing store");
		restore(ps);
		return SYSERR;
	}
	
	proctab[currpid].bs_map[bs_id].bs_status = BSM_MAPPED;
	proctab[currpid].bs_map[bs_id].bs_npages = npages;
	proctab[currpid].bs_map[bs_id].shared = bs_shared;
	restore(ps);
	return bsm_tab[bs_id].bs_npages;
}