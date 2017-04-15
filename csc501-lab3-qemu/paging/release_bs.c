#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {
/*
xyu10: A backing store is freed when all processes that share it have called release_bs.
https://classic.wolfware.ncsu.edu/wrap-bin/mesgboard/csc:501::001:1:2017?task=ST&Forum=4&Topic=21
*/
  /* release the backing store with ID bs_id */
   if(bs_id <0 || bs_id > 7)
	   return SYSERR;
   int i, ct;
    proctab[currpid].bs_map[bs_id].bs_status = BSM_UNMAPPED;
	proctab[currpid].bs_map[bs_id].bs_vpno = -1;
	proctab[currpid].bs_map[bs_id].bs_npages = -1;
	proctab[currpid].bs_map[bs_id].shared = TRUE;
	
	for(i=0, ct=0; i < MAX_PROCESS_PER_BS; i++)
	{
		if(bsm_tab[store].pr_map[i].bs_pid == pid || if(bsm_tab[store].pr_map[i].bs_pid == -1)
		{
			bsm_tab[store].pr_map[i].bs_pid = -1;
			bsm_tab[store].pr_map[i].bs_vpno = -1;
			ct++;
		}
	}
	if(ct == MAX_PROCESS_PER_BS)
	{
		// All the processes have released this BS_ID
		bsm_tab[store].bs_status = BSM_UNMAPPED;
		bsm_tab[store].bs_npages = -1;
		bsm_tab[store].bs_npages = -1;
		bsm_tab[store].bs_sem = -1;
		bsm_tab[store].shared = TRUE;
	}
   return OK;
}
