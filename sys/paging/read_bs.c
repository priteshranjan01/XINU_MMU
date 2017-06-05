#include <conf.h>
#include <kernel.h>
#include <mark.h>
#include <bufpool.h>
#include <proc.h>
#include <paging.h>

SYSCALL read_bs(char *dst, bsd_t bs_id, int page) {

  /* fetch page page from map map_id
     and write beginning at dst.
  */
   void * phy_addr = BACKING_STORE_BASE + bs_id*BACKING_STORE_UNIT_SIZE + page*NBPG;
   bcopy(phy_addr, (void*)dst, NBPG);
   if(debug) kprintf("\n Inserting from bs_fr_tab bs_id=%d, pageth=%d frame_no %d",bs_id, page, (int)dst>>12);

   insert_bs_fr_tab_info(bs_id, page, (int)dst>>12);
}


