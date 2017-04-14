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
  {		// 4 Frames for the global page tables.
	  frm_tab[i].fr_status = FRM_MAPPED;
	  frm_tab[i].fr_pid = 0;
	  frm_tab[i].fr_vpno = ENTRIES_PER_PAGE + i;
	  frm_tab[i].fr_refcnt = 1;
	  frm_tab[i].fr_type = FR_TBL;
	  frm_tab[i].fr_dirty = FALSE;
	  frm_tab[i].next = -1;
  }
  for(i=4; i< 4+NPROC; i++)
  {		// Next NPROC frames for PD's of processes.
	  frm_tab[i].fr_status = FRM_MAPPED;
	  frm_tab[i].fr_pid = i-4;
	  frm_tab[i].fr_vpno = ENTRIES_PER_PAGE + i;
	  frm_tab[i].fr_refcnt = 1;
	  frm_tab[i].fr_type = FR_DIR;
	  frm_tab[i].fr_dirty = FALSE;
	  frm_tab[i].next = -1;
  }
  for (i=4+NPROC; i < FRAME0-ENTRIES_PER_PAGE ; i++)
  {	// Frames for page tables.
	  frm_tab[i].fr_status = FRM_UNMAPPED;
	  frm_tab[i].fr_pid = -1;
	  frm_tab[i].fr_vpno = -1;
	  frm_tab[i].fr_refcnt = -1;
	  frm_tab[i].fr_type = FR_TBL;
	  frm_tab[i].fr_dirty = FALSE;
	  frm_tab[i].next = -1;
	  //TODO: Add lines fr_map_t structure is changed
  }
  for(; i< FRAME0 + NFRAMES - ENTRIES_PER_PAGE; i++)
  {		// Available frames for user data.
	  frm_tab[i].fr_status = FRM_UNMAPPED;
	  frm_tab[i].fr_pid = -1;
	  frm_tab[i].fr_vpno = -1;
	  frm_tab[i].fr_refcnt = -1;
	  frm_tab[i].fr_type = FR_PAGE;
	  frm_tab[i].fr_dirty = FALSE;
	  frm_tab[i].next = -1;  
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
	/* Will return a value between 1028 to 1077 both inclusive*/
	if ((frm_tab[pid+4].fr_pid != pid) && frm_tab[pid+4].fr_type != FR_DIR)
	{
		kprintf("\nThe inverted page table has not been initialized properly");
		kill(currpid);
		return SYSERR;
	}
	*frame_number = frm_tab[pid+4].fr_vpno;
	frm_tab[pid+5].fr_status = FRM_MAPPED;
	return OK;
}

SYSCALL get_frame_for_PT(int pid, int *frame_number)
{
	int i;
	for (i=4+NPROC; i < FRAME0-ENTRIES_PER_PAGE ; i++)
	{
		if(frm_tab[i].fr_status == FRM_UNMAPPED)
		{
			frm_tab[i].fr_pid = currpid;
			*frame_number = i;
			return OK;
		}
	}
	kprintf("\nRan out of frames for page tables");
	return SYSERR;
}
SYSCALL get_frm(int* frame_number)
{
	/* Will give a value in between 1030 and 2047, both inclusive */
  //kprintf("To be implemented!\n");
  // First look for an unmapped frame.
  int i;
  
  for(i= FRAME0 - ENTRIES_PER_PAGE; i <  FRAME0 - ENTRIES_PER_PAGE + NFRAMES; i++)
  {
	if (frm_tab[i].fr_status == FRM_UNMAPPED)
	{
		frm_tab[i].fr_status = FRM_MAPPED;
		*frame_number = i+ ENTRIES_PER_PAGE;
		return OK;
	}
  }
  printf("\n\nCouldn't find an unmapped frame\n\n");
  return SYSERR;
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
