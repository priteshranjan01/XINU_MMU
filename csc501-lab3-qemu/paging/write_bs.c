#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <mark.h>
#include <bufpool.h>
#include <paging.h>

int write_bs(char *src, bsd_t bs_id, int page) {

  /* write one page of data from src
     to the backing store bs_id, page
     page.
  */
  int status;
   char * phy_addr = BACKING_STORE_BASE + bs_id*BACKING_STORE_UNIT_SIZE + page*NBPG;
   bcopy((void*)src, phy_addr, NBPG);
   kprintf("\nremoving from bs_fr_tab bs_id=%d, pageth=%d frame_no %d",bs_id, page, (int)src>>12);
	status = remove_bs_fr_tab_info(bs_id, page, (int)src>>12);
	if(status == SYSERR)
		kprintf("\nError while removing from bs_fr_tab bs_id=%d, pageth=%d",bs_id, page);
}

