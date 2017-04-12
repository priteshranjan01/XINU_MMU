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
  STATWORD ps;
  int i ;
  disable(ps);
  kprintf("\n\n\nWhere is my Mind\n\n");
  for(i=0; i< 4; i++)
  {
	  frm_tab[i].fr_status = FRM_MAPPED;
	  frm_tab[i].fr_pid = 0;
	  frm_tab[i].fr_vpno = ENTRIES_PER_PAGE + i;
	  frm_tab[i].fr_refcnt = 1;
	  frm_tab[i].fr_type = FR_TBL;
	  frm_tab[i].fr_dirty = FALSE;
	  frm_tab[i].next = -1;
  }
  for(i=4; i< 54; i++)
  {
	  frm_tab[i].fr_status = FRM_MAPPED;
	  frm_tab[i].fr_pid = i-4;
	  frm_tab[i].fr_vpno = ENTRIES_PER_PAGE + i;
	  frm_tab[i].fr_refcnt = 1;
	  frm_tab[i].fr_type = FR_DIR;
	  frm_tab[i].fr_dirty = FALSE;
	  frm_tab[i].next = -1;
  }
  for (i=54; i < 54+NFRAMES; i++)
  {
	  frm_tab[i].fr_status = FRM_UNMAPPED;
	  frm_tab[i].fr_pid = -1;
	  frm_tab[i].fr_vpno = -1;
	  frm_tab[i].fr_refcnt = -1;
	  frm_tab[i].fr_type = FR_PAGE;
	  frm_tab[i].fr_dirty = FALSE;
	  frm_tab[i].next = -1;
	  //TODO: Add lines fr_map_t structure is changed
  }
  restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
 
SYSCALL get_frame_for_PD(int pid, int * frame_number)
{
	/* Will return a value between 1028 to 1077 */
	if ((frm_tab[pid+4].fr_pid != pid) && frm_tab[pid+4].fr_type != FR_DIR)
	{
		kprintf("\nThe inverted page table has not been initialized properly");
		return SYSERR;
	}
	*frame_number = frm_tab[pid+4].fr_vpno;
	frm_tab[pid+5].fr_status = FRM_MAPPED;
	return OK;
}

SYSCALL get_frm(int* avail)
{
	/* Will give a value in between 1030 and 2047, both inclusive */
  kprintf("To be implemented!\n");
  // First look for an unmapped frame.
  int i= FRAME0 - ENTRIES_PER_PAGE;
  for(; i < NFRAMES; i++)
  {
	if (frm_tab[i].fr_status == FRM_UNMAPPED)
	{
		frm_tab[i].fr_status = FRM_MAPPED;
		*avail = i+1024;
		return OK;
	}
  }
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
