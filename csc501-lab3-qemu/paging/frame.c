/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
  //kprintf("To be implemented!\n");
  STATWORD ps;
  int i ;
  disable(ps);
  for (i=0; i < NFRAMES; i++)
  {
	  frm_tab[i].fr_status = FRM_UNMAPPED;
	  frm_tab[i].fr_pid = -1;
	  frm_tab[i].fr_vpno = -1;
	  frm_tab[i].fr_refcnt = -1;
	  frm_tab[i].fr_type = FR_PAGE;
	  frm_tab[i].fr_dirty = FALSE;
	  //TODO: Add lines fr_map_t structure is changed
  }
  restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
  kprintf("To be implemented!\n");
  return OK;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{

  kprintf("To be implemented!\n");
  return OK;
}
