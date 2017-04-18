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
	  frm_tab[i].fr_vpno = -1;  // These frames don't keep a virtual address page.
	  frm_tab[i].fr_refcnt = 1;
	  frm_tab[i].fr_type = FR_TBL;
	  frm_tab[i].fr_dirty = FALSE;
	  frm_tab[i].next = -1;
	  frm_tab[i].ctr = 0UL;
  }
  for(i=4; i< 4+NPROC; i++)  // Loop from i = 4 to 53
  {		// Next NPROC frames for PD's of processes.
	  frm_tab[i].fr_status = FRM_MAPPED;
	  frm_tab[i].fr_pid = i-4;
	  frm_tab[i].fr_vpno = -1;	// These frames don't keep a virtual address page.
	  frm_tab[i].fr_refcnt = 1;
	  frm_tab[i].fr_type = FR_DIR;
	  frm_tab[i].fr_dirty = FALSE;
	  frm_tab[i].next = -1;
	  frm_tab[i].ctr = 0UL;
  }
  for (i=4+NPROC; i < FRAME0-ENTRIES_PER_PAGE ; i++)  // Loop from 54 to 511
  {	// Frames for page tables.
	  frm_tab[i].fr_status = FRM_UNMAPPED;
	  frm_tab[i].fr_pid = -1;
	  frm_tab[i].fr_vpno = -1;
	  frm_tab[i].fr_refcnt = -1;
	  frm_tab[i].fr_type = FR_TBL;
	  frm_tab[i].fr_dirty = FALSE;
	  frm_tab[i].next = -1;
	  frm_tab[i].ctr = 0UL;
	  //TODO: Add lines fr_map_t structure is changed
  }
  for(; i< FRAME0 + NFRAMES - ENTRIES_PER_PAGE; i++)  //Loop from 512 to 1024
  {		// Available frames for user data.
	  frm_tab[i].fr_status = FRM_UNMAPPED;
	  frm_tab[i].fr_pid = -1;
	  frm_tab[i].fr_vpno = -1;
	  frm_tab[i].fr_refcnt = -1;
	  frm_tab[i].fr_type = FR_PAGE;
	  frm_tab[i].fr_dirty = FALSE;
	  frm_tab[i].next = -1;  
	  frm_tab[i].ctr = 0UL;
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
	STATWORD ps;
	disable(ps);
	if ((frm_tab[pid+4].fr_pid != pid) && frm_tab[pid+4].fr_type != FR_DIR)
	{
		kprintf("\nThe inverted page table has not been initialized properly");
		restore(ps);
		kill(currpid);
		return SYSERR;
	}
	*frame_number = 4+pid+ENTRIES_PER_PAGE;
	frm_tab[pid+5].fr_status = FRM_MAPPED;
	restore(ps);
	return OK;
}

SYSCALL get_frame_for_PT(int *frame_number)
{
	//NOTE: Disable interrupts before calling this method.
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


int clean_up_inverted_page_table(int pid)
{
	// Free up the entries in inverted page table.
	// Iterates through SC queue. If any frame of process pid is found or of a process which 
	// is dead then frees that frame.
	int p, i, nelements;
	int queue[NFRAMES];
	
	if(sc_head == -1)  return OK;
	if(debug) kprintf("\nCleanup called for pid # %d, sc_head = %d\n", pid, p);
	
	p = sc_head;
	for(i=0; i<NFRAMES; i++)
	{
		queue[i] = p;
		p = frm_tab[p].next;
		if(p == sc_head)
			break;
	}
	nelements = i;
	if(debug) print_sc_queue();
	
	for(i = 0; i<=nelements; i++)
	{
		p = queue[i];
		if (debug) kprintf("\nfr_pid = %d, process state = %d, frame status=%d, vpno=0x%x, type=%d, dirty=%d",frm_tab[p].fr_pid, proctab[frm_tab[p].fr_pid].pstate, frm_tab[p].fr_status, frm_tab[p].fr_vpno, frm_tab[p].fr_type, frm_tab[p].fr_dirty);
		if(frm_tab[p].fr_pid == pid || proctab[frm_tab[p].fr_pid].pstate == PRFREE)
		{
		  free_frm(p+ENTRIES_PER_PAGE);
		  remove_from_sc_queue(p);
		}
	}
	if(debug) print_sc_queue();
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
		// switch(grpolicy())
		// {
			// case AGING:
			// default: /* Fall Through */
			// case SC: 
				frm_tab[i].ctr = 0UL;		/* Initialize the ctr used by AGING PR policy to zero */
				status = insert_into_sc_queue(i);  /* Expects a value in range 512 to 1023 both inclusive*/
		
		// }
		return OK;
	}
  }
  if (debug) kprintf("\nCouldn't find an unmapped frame. Going to look for a victim\n\n");
  
  status = get_victim_frame(frame_number);
  return status;
}

int get_victim_frame(int * frame_number)
{
	int status, is_dirty, store, pageth, pid;
	unsigned long vpno;
	switch(grpolicy())
	{
		case AGING:
			status = get_AGING_policy_victim(frame_number, &is_dirty, &vpno, &pid);
			break;
		default: /* Fall Through */
		case SC: 
			status = get_SC_policy_victim(frame_number, &is_dirty, &vpno, &pid);
	}
	if(status == SYSERR)
	{
			kprintf("\n PR policy %d returned SYSERR", grpolicy());
			return status;
	}
	if(debug) kprintf("\n frame_number %d, is_dirty= %d, vpno= 0x%08x, pid= 0x%08x",frame_number, is_dirty, vpno, pid);
	if(is_dirty)
	{
		if(debug) kprintf("\nDirty bit was set Write to backing store. ");
		status = bsm_lookup(pid, vpno<<12, &store, &pageth);
		if(debug) kprintf("\npid %d, store %d, pageth %d vpno=0x%x",pid, store, pageth, vpno);
		if(status == SYSERR)
		{
			kprintf("\nBsm lookup failed while trying to get victim frame. Swapping out to BS aborted");	
		}
		else
		{write_bs((char *)((*frame_number)<<12), store, pageth);}
	}
 if(pr_debug) kprintf("\nSwapped out Frame number # %d",*frame_number);
 return OK;
}

int get_AGING_policy_victim(int * frame_number, int * is_dirty, unsigned long * vpno1,int *pid1)
{  // Sorry, but I had to duplicate the code.
	int ct=0, p;
	unsigned long pdbr, pd_off, pt_off;
	unsigned long min_ctr= 0xffffffff;
	unsigned long *vpno ;
	for(p = sc_head; ct <= NFRAMES; ct++)
	{	
		if(sc_head == -1)
		{
			kprintf("\nget_AGING_policy_victim: SC queue is not set sc_head %d",sc_head);
			return SYSERR;
		}
		
		*pid = frm_tab[p].fr_pid;
		*vpno = frm_tab[p].fr_vpno;
		pdbr = ((proctab[*pid].pdbr)>>12)<<12;  // Clear the 12 LSB bits.
		pd_off = ((*vpno) <<12) >> 22;  // Double check this. vpno shall have only 20 LSB bits.
		pt_off = ((*vpno) << 22) >> 22; 
		if(debug) kprintf("\nDouble Check these values: pdbr = 0x%x, vpno = 0x%08x pd_off 0x%x pt_off 0x%x ",pdbr,*vpno, pd_off,pt_off);
		pd_off = pd_off << 2;  // Multiply by 4
		pt_off = pt_off << 2;
		pd_t *pde = (pd_t*)(pdbr + pd_off);
		if ((*pde).pde.pd_pres == 0)
			{kprintf("\nSTAGE 1: We are in deep trouble."); return SYSERR;}
		pt_t *pte = (pt_t*)(((*pde).pde.pd_base << 12) + pt_off);
		if ((*pte).pte.pt_pres == 0)
			{kprintf("\nSTAGE 2: We are in deep trouble."); return SYSERR;}
		frm_tab[p].ctr = frm_tab[p].ctr >> 1 ;
		if((*pte).pte.pt_acc == 1)
		{
			SET_BIT(frm_tab[p].ctr, 31);
		}
		else
		{
			RESET_BIT(frm_tab[p].ctr, 31);
		}
		if(frm_tab[p].ctr < min_ctr)
		{
			min_ctr = frm_tab[p].ctr;
			*frame_number = p + ENTRIES_PER_PAGE;
			*is_dirty = (*pte).pte.pt_dirty;
			*vpno1 = *vpno;
			*pid1 = *pid;
		}
		p = frm_tab[p].next;
		if(p == sc_head)
			break;
	}
	if(ct > NFRAMES);
	{
		kprintf("The queue has not been set up properly");
		return SYSERR;
	}
	// Move the sc_head one step forward to give it the illusion of removing and inserting the frame.
	sc_head = frm_tab[sc_head].next;
	return OK;
}


int get_SC_policy_victim(int * frame_number, int * is_dirty, unsigned long * vpno,int *pid)
{
	/*
	Returns the frame to be swapped out. THe frames owner pid. is_dirty frame and the virtual pages that it maps.
	--> frame_number tells the next victim.
	--> is_dirty tells if the page is dirty.
	This procedure also invalidates the PTE entry.
	Reduces the reference_count in the inverted page table. 
		If the reference_count becomes zero then, invalidates the PDE entry.
	Removes the victim frame from SC queue.
	*/
	int ct=0;
	unsigned long pdbr, pd_off, pt_off;
	for(; ct <= NFRAMES; ct++)
	{	
		if(sc_head == -1)
		{
			kprintf("\nget_SC_policy_victim: SC queue is not set sc_head %d",sc_head);
			return SYSERR;
		}
		*pid = frm_tab[sc_head].fr_pid;
		*vpno = frm_tab[sc_head].fr_vpno;
		pdbr = ((proctab[*pid].pdbr)>>12)<<12;  // Clear the 12 LSB bits.
		pd_off = ((*vpno) <<12) >> 22;  // Double check this. vpno shall have only 20 LSB bits.
		pt_off = ((*vpno) << 22) >> 22; 
		if(debug) kprintf("\nDouble Check these values: pdbr = 0x%x, vpno = 0x%08x pd_off 0x%x pt_off 0x%x ",pdbr,*vpno, pd_off,pt_off);
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
			// See if we need to remove from the queue. Currently, I think we don't.
			sc_head = frm_tab[sc_head].next;
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
SYSCALL free_frm(int frame_no)
{

//  kprintf("To be implemented!\n");
  /*
  Expects a value x ,  1024 <= x < 2048
  */
  STATWORD ps;
  disable(ps);
  int i;
  pt_t * addr = (pt_t*)(frame_no * NBPG);
  pd_t * addr1 = (pt_t*)(frame_no * NBPG);
  if(debug) kprintf("\nfree_frm called with frame_no # %d",frame_no);
  frame_no -= ENTRIES_PER_PAGE;
  if(frame_no< 4+NPROC)
  {		if(debug) kprintf("\n4 Frames for the global page tables and PD can't be freed.");
		//DO nothing, Keep silent. return OK;
  }
  else if (frame_no >= 4+NPROC && frame_no < FRAME0-ENTRIES_PER_PAGE)  // 54 to 511
  {	// Frames for page tables.
	  frm_tab[frame_no].fr_status = FRM_UNMAPPED;
	  frm_tab[frame_no].fr_pid = -1;
	  frm_tab[frame_no].fr_vpno = -1;
	  frm_tab[frame_no].fr_refcnt = -1;
	  frm_tab[frame_no].fr_type = FR_TBL;
	  frm_tab[frame_no].fr_dirty = FALSE;
	  frm_tab[frame_no].ctr = 0UL;
	  //frm_tab[frame_no].next = -1;
	  //Invalidate the entries in the PT this frame holds.
	  for(i=0; i < ENTRIES_PER_PAGE; i++)
	  {
		  addr[i].dummy = 0;
	  }
  }
  else if(frame_no >= FRAME0-ENTRIES_PER_PAGE && frame_no < FRAME0 + NFRAMES - ENTRIES_PER_PAGE)  // 512 to 1024
  {		// Available frames for user data.
	  frm_tab[frame_no].fr_status = FRM_UNMAPPED;
	  frm_tab[frame_no].fr_pid = -1;
	  frm_tab[frame_no].fr_vpno = -1;
	  frm_tab[frame_no].fr_refcnt = -1;
	  frm_tab[frame_no].fr_type = FR_PAGE;
	  frm_tab[frame_no].fr_dirty = FALSE;
	  frm_tab[frame_no].ctr = 0UL;
	  //frm_tab[frame_no].next = -1;  
	}
	else
	{
		restore(ps);
		return SYSERR;
	}

  restore(ps);
  return OK;
}

