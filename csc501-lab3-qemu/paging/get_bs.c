#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {

	/* requests a new mapping of npages with ID map_id */
	/*
	
	*/
    if (bs_id < 0 || bs_id > BS_COUNT || npages <= 0 || npages > 256)
	{
		kprintf("\nInvalid inputs to get_bs procedure");
		return SYSERR;
	}
	
	if (is_bsm_available(bs_id, currpid) == FALSE)
	{
		kprintf("\nBS # %d not available for process ID %d",bs_id, currpid);
		return SYSERR;
	}
	if (bsm_tab[bs_id].bs_status == BSM_MAPPED)
	{
		return bsm_tab[bs_id].bs_npages;
	}
	
	bsm_tab[bs_id].bs_status = BSM_MAPPED;
	bsm_tab[bs_id].bs_npages = npages;
	bsm_tab[bs_id].shared = TRUE;
	// bsm_tab[bs_id].pr_map[0].bs_pid = currpid;  // Here or in bsm_map?
	
	proctab[currpid].bs_map[bs_id].bs_status = BSM_MAPPED;
	proctab[currpid].bs_map[bs_id].bs_npages = npages;
	proctab[currpid].bs_map[bs_id].shared = TRUE;
	
	return npages;
}