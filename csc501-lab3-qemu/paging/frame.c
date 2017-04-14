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

SYSCALL get_frame_for_PT(int *frame_number)
{
	int i;
	for (i=4+NPROC; i < FRAME0-ENTRIES_PER_PAGE ; i++)
	{
		if(frm_tab[i].fr_status == FRM_UNMAPPED)
		{
			frm_tab[i].fr_pid = currpid;
			*frame_number = i+ ENTRIES_PER_PAGE;
			return OK;
		}
	}
	kprintf("\nRan out of frames for page tables");
	return SYSERR;
}

int insert_into_sc_queue(int frame_no)
{
	if (frame_no < 512 || frame_no > 512 + NFRAMES)
	{
		kprintf("\n Invalid Frame # %d Range of value is 512 to %d", frame_no, 512 + NFRAMES);
		return SYSERR;
	}
	if(sc_head == -1)
	{
		sc_head = sc_tail = frame_no;
		frm_tab[sc_head].next = sc_head;
	}
	else
	{
		frm_tab[frame_no].next = sc_head;
		frm_tab[sc_tail].next = frame_no;
		sc_tail = frame_no;
	}
	return OK;
}
SYSCALL get_frm(int* frame_number)
{
	/* Will give a value in between 1030 and 2047, both inclusive */
  //kprintf("To be implemented!\n");
  // First look for an unmapped frame.
  int i, status;
  
  for(i= FRAME0 - ENTRIES_PER_PAGE; i <  FRAME0 - ENTRIES_PER_PAGE + NFRAMES; i++)
  {
	if (frm_tab[i].fr_status == FRM_UNMAPPED)
	{
		frm_tab[i].fr_status = FRM_MAPPED;
		frm_tab[i].fr_type = FR_PAGE;
		*frame_number = i+ ENTRIES_PER_PAGE;
		switch(grpolicy())
		{
			case AGING:
			default: /* Fall Through */
			case SC: 
				status = insert_into_sc_queue(i);  /* Expects a value in range 512 to 1023 both inclusive*/
		}
		return OK;
	}
  }
  if (debug) kprintf("\nCouldn't find an unmapped frame. Going to look for a victim\n\n");
  
  status = get_victim_frame(frame_number);
  return status;
}

int get_victim_frame(int * frame_number)
{
	int status, is_dirty, store, pageth;
	unsigned long vpno;
	switch(grpolicy())
	{
		case AGING:
		default: /* Fall Through */
		case SC: 
			status = get_SC_policy_victim(frame_number, &is_dirty, &vpno);
			if(is_dirty)
			{
				// Write to backing store.
				bsm_lookup(currpid, vpno<<12, &store, &pageth);
				if(debug) kprintf("\ncurrpid %d, store %d, pageth %d",currpid, store, pageth);
				write_bs((char *)((*frame_number)<<12), store, pageth);
			}
	}
 return OK;
}

int get_SC_policy_victim(int * frame_number, int * is_dirty, unsigned long * vpno)
{
	/*
	--> frame_number tells the next victim.
	--> is_dirty tells if the page is dirty.
	This procedure also invalidates the PTE entry.
	Reduces the reference_count in the inverted page table. 
		If the reference_count becomes zero then, invalidates the PDE entry.
	Removes the victim frame from SC queue.
	*/
	int ct=0, pid;
	unsigned long pdbr, pd_off, pt_off;
	for(; ct <= NFRAMES; ct++)
	{
		pid = frm_tab[sc_head].fr_pid;
		*vpno = frm_tab[sc_head].fr_vpno;
		pdbr = ((proctab[pid].pdbr)>>12)<<12;  // Clear the 12 LSB bits.
		pd_off = ((*vpno) <<12) >> 22;  // Double check this. vpno shall have only 20 LSB bits.
		pt_off = ((*vpno) << 22) >> 22; 
		if(debug) kprintf("\npdbr = 0x%x, pd_off 0x%x pt_off 0x%x ",pdbr,pd_off,pt_off);
		pd_off = pd_off << 2;  // Multiply by 4
		pt_off = pt_off << 2;
		pd_t *pde = (pd_t*)(pdbr + pd_off);
		if ((*pde).pde.pd_pres == 0)
			{kprintf("\nSTAGE 1: We are in deep trouble."); return SYSERR;}
		pt_t *pte = (pt_t*)(((*pde).pde.pd_base << 12) + pt_off);
		if ((*pte).pte.pt_pres == 0)
			{kprintf("\nSTAGE 2: We are in deep trouble."); return SYSERR;}
		if((*pte).pte.pt_acc == 1)
		{
			if(debug) kprintf("\nThe page has been accessed");
			(*pte).pte.pt_acc = 0;  // Set the accessed bit to 0 and move on.
			sc_tail = sc_head;
			sc_head = frm_tab[sc_head].next;
		}
		else
		{
			if(debug) kprintf("\nFound the victim. Frame # %d is gonna DIE",sc_head);
			*is_dirty = (*pte).pte.pt_dirty;
			*frame_number = sc_head + ENTRIES_PER_PAGE;
			(*pte).dummy = 0; // Invalidate the PTE entry.
			// TODO: Decrease the ref_cnt from inverted page table. if it becomes zero then invalidate the PDE.
			// TODO: Invalidate the TLB entry.
			// Remove the frame from the queue.
			// See if we need to remove the queue. Currently, I think we don't.
			sc_tail = sc_head;
			sc_head = frm_tab[sc_head].next;
			//frm_tab[sc_tail].next = sc_head;
			return OK;
		}
	}
	// If the code reaches here, this means the circular queue is not properly set or something is smelling.
	// A good day to die hard.
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
