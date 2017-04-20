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
  int i,j ;
  disable(ps);
  for(i=0; i< 4; i++)
  {		// 4 Frames for the global page tables.
	  frm_tab[i].fr_status = FRM_MAPPED;
	for(j=0; j< MAX_PROCESS_PER_BS; j++)
	{
		frm_tab[i].pr_map[j].bs_pid = 0;  
		frm_tab[i].pr_map[j].bs_vpno = -1;  // These frames don't keep a virtual address page.
	}
	  frm_tab[i].fr_refcnt = 1;
	  frm_tab[i].fr_type = FR_TBL;
	  frm_tab[i].fr_dirty = FALSE;
	  frm_tab[i].next = -1;
	  frm_tab[i].ctr = 0;
	  frm_tab[i].shared = TRUE;
	  frm_tab[i].bs_id = -1;
	  frm_tab[i].pageth = -1;
  }
  for(i=4; i< 4+NPROC; i++)  // Loop from i = 4 to 53
  {		// Next NPROC frames for PD's of processes.
	  frm_tab[i].fr_status = FRM_MAPPED;
	for(j=0; j< MAX_PROCESS_PER_BS; j++)
	{
		frm_tab[i].pr_map[j].bs_pid = i-4;  
		frm_tab[i].pr_map[j].bs_vpno = -1;  // These frames don't keep a virtual address page.
	}
	  frm_tab[i].fr_refcnt = 1;
	  frm_tab[i].fr_type = FR_DIR;
	  frm_tab[i].fr_dirty = FALSE;
	  frm_tab[i].next = -1;
	  frm_tab[i].ctr = 0;
	  frm_tab[i].shared = FALSE;
	  frm_tab[i].bs_id = -1;
	  frm_tab[i].pageth = -1;
  }
  for (i=4+NPROC; i < FRAME0-ENTRIES_PER_PAGE ; i++)  // Loop from 54 to 511
  {	// Frames for page tables.
	  frm_tab[i].fr_status = FRM_UNMAPPED;
	for(j=0; j< MAX_PROCESS_PER_BS; j++)
	{
		frm_tab[i].pr_map[j].bs_pid = -1;  
		frm_tab[i].pr_map[j].bs_vpno = -1;  // These frames don't keep a virtual address page.
	}
	  frm_tab[i].fr_refcnt = -1;
	  frm_tab[i].fr_type = FR_TBL;
	  frm_tab[i].fr_dirty = FALSE;
	  frm_tab[i].next = -1;
	  frm_tab[i].ctr = 0;
	  frm_tab[i].shared = FALSE;
	  frm_tab[i].bs_id = -1;
	  frm_tab[i].pageth = -1;
  }
  for(; i< FRAME0 + NFRAMES - ENTRIES_PER_PAGE; i++)  //Loop from 512 to 1024
  {		// Available frames for user data.
	  frm_tab[i].fr_status = FRM_UNMAPPED;
	for(j=0; j< MAX_PROCESS_PER_BS; j++)
	{
		frm_tab[i].pr_map[j].bs_pid = -1;  
		frm_tab[i].pr_map[j].bs_vpno = -1;  // These frames don't keep a virtual address page.
	}
	  frm_tab[i].fr_refcnt = -1;
	  frm_tab[i].fr_type = FR_PAGE;
	  frm_tab[i].fr_dirty = FALSE;
	  frm_tab[i].next = -1;  
	  frm_tab[i].ctr = 0;
	  frm_tab[i].shared = TRUE;
	  frm_tab[i].bs_id = -1;
	  frm_tab[i].pageth = -1;
	}
  restore(ps);
  return OK;
}


/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int frame_no, int pid)
{

  /*
  Expects a value x ,  1024 <= x < 2048
  */
  STATWORD ps;
  disable(ps);
  int i, j, ct=0;
  pt_t * addr = (pt_t*)(frame_no * NBPG);

  if(debug) kprintf("\nfree_frm called with frame_no # %d",frame_no);
  frame_no -= ENTRIES_PER_PAGE;
  if(frame_no< 4+NPROC)
  {		if(debug) kprintf("\n4 Frames for the global page tables and PD can't be freed.");
		//DO nothing, Keep silent. return OK;
  }
  else if (frame_no >= 4+NPROC && frame_no < FRAME0-ENTRIES_PER_PAGE)  // 54 to 511
  {	// Frames for page tables.
	  frm_tab[frame_no].fr_status = FRM_UNMAPPED;
	  for(j=0; j< MAX_PROCESS_PER_BS; j++)
	 {
		frm_tab[frame_no].pr_map[j].bs_pid = -1;  
		frm_tab[frame_no].pr_map[j].bs_vpno = -1;  // These frames don't keep a virtual address page.
	 }
	  frm_tab[frame_no].fr_refcnt = -1;
	  frm_tab[frame_no].fr_type = FR_TBL;
	  frm_tab[frame_no].fr_dirty = FALSE;
	  frm_tab[frame_no].ctr = 0;
	  frm_tab[i].shared = TRUE;
	  frm_tab[i].bs_id = -1;
	  frm_tab[i].pageth = -1;
	  //Invalidate the entries in the PT this frame holds.
	  for(i=0; i < ENTRIES_PER_PAGE; i++)
	  {
		  addr[i].dummy = 0;
	  }
  }
  else if(frame_no >= FRAME0-ENTRIES_PER_PAGE && frame_no < FRAME0 + NFRAMES - ENTRIES_PER_PAGE)  // 512 to 1024
  {		// Available frames for user data.
	  
//	  frm_tab[frame_no].fr_status = FRM_UNMAPPED;
	ct = 0;
//	kprintf("\nABRA KA DABRA");
	for(j=0; j< MAX_PROCESS_PER_BS; j++)
	 {
		 //kprintf("\n %d %d ",frm_tab[frame_no].pr_map[j].bs_pid, pid);
		 if((frm_tab[frame_no].pr_map[j].bs_pid == pid) || 
				(frm_tab[frame_no].pr_map[j].bs_pid == -1))
			{
				frm_tab[frame_no].pr_map[j].bs_pid = -1;  
				frm_tab[frame_no].pr_map[j].bs_vpno = -1; 
				ct++;
			}
	 }
	  frm_tab[frame_no].fr_refcnt = -1;
	  frm_tab[frame_no].fr_type = FR_PAGE;
	  frm_tab[frame_no].fr_dirty = FALSE;
	  frm_tab[frame_no].ctr = 0;
	  frm_tab[i].shared = TRUE;
	  frm_tab[i].bs_id = -1;
	  frm_tab[i].pageth = -1;
	  if(ct >= MAX_PROCESS_PER_BS)
		  frm_tab[frame_no].fr_status = FRM_UNMAPPED;
	
	}
	else
	{
		restore(ps);
		return SYSERR;
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
	if ((frm_tab[pid+4].pr_map[0].bs_pid != pid) && frm_tab[pid+4].fr_type != FR_DIR)
	{  // Since PD and PTs shall not be shared we can 0 as the fixed index in pr_map
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
	{// Since PD and PTs shall not be shared we can 0 as the fixed index in pr_map
		if(frm_tab[i].fr_status == FRM_UNMAPPED)
		{
			frm_tab[i].pr_map[0].bs_pid = currpid;
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
	int p, i, j, nelements;
	int queue[NFRAMES];
	int remove_from_queue = TRUE;
	if(sc_head == -1)  return OK;
	p = sc_head;

	if(debug) kprintf("\nCleanup called for pid # %d, sc_head = %d\n", pid, p);
	
	for(i=0; i<NFRAMES; i++)
	{
		queue[i] = p;
		p = frm_tab[p].next;
		if(p == sc_head)
			break;
	}
	nelements = i;
	if(debug) print_sc_queue();
	int temp_pid = -1;
	for(i = 0; i<=nelements; i++)
	{	remove_from_queue = TRUE;
		p = queue[i];
		if (debug) kprintf("\n frame status=%d, type=%d is_dirty= %d", frm_tab[p].fr_status, frm_tab[p].fr_type, frm_tab[p].fr_dirty);
		for(j=0; j<MAX_PROCESS_PER_BS; j++)
		{
			temp_pid = frm_tab[p].pr_map[j].bs_pid;
			if( temp_pid == -1)
				continue;
			if(temp_pid != pid && proctab[temp_pid].pstate != PRFREE)
			{
				// Some other LIVE process is holding this frame. Don't remove this frame
				// from queue. 
				remove_from_queue = FALSE;
			}
				
			if (debug) kprintf("\nfr_pid = %d, process state = %d, vpno=0x%x ",
					temp_pid, proctab[temp_pid].pstate, frm_tab[p].pr_map[j].bs_vpno);

			if(temp_pid == pid || proctab[temp_pid].pstate == PRFREE)
			{
				free_frm(p+ENTRIES_PER_PAGE, temp_pid);	
			    if(debug) kprintf("\n Frame No %d is_dirty=%d, Writing to memory", p+ENTRIES_PER_PAGE, frm_tab[p].fr_dirty);
  			    write_bs((p+ENTRIES_PER_PAGE)<<12, frm_tab[p].bs_id, frm_tab[p].pageth);
			}
		}
		if(remove_from_queue == TRUE) 
			remove_from_sc_queue(p);
	}
	if(debug) print_sc_queue();
	return OK;
}



SYSCALL get_frm(int* frame_number)
{
	/* Will give a value in between 1030 and 2047, both inclusive */
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
				frm_tab[i].ctr = 0;		/* Initialize the ctr used by AGING PR policy to zero */
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
			//status = get_AGING_policy_victim(frame_number, &is_dirty, &vpno, &pid);
			//break;
		default: /* Fall Through */
		case SC: 
			status = get_SC_policy_victim(frame_number, &is_dirty, &vpno, &pid);
	}
	if(status == SYSERR)
	{
			kprintf("\n PR policy %d returned SYSERR", grpolicy());
			return status;
	}
	if(debug) kprintf("\n frame_number %d, is_dirty= %d, vpno= 0x%08x, pid= %d",*frame_number, is_dirty, vpno, pid);
	if(is_dirty)
	{
		if(debug) kprintf("\nDirty bit was set Write to backing store. ");
		// TODO: THIS WILL HAVE TO CHANGE
		//status = bsm_lookup(pid, vpno<<12, &store, &pageth);
		status = get_bs_offset(*frame_number, &store, &pageth);
		if(debug) kprintf("\npid %d, store %d, pageth %d vpno=0x%x",pid, store, pageth, vpno);
		if(status == SYSERR)
		{
			kprintf("\nBsm lookup failed while trying to get victim frame. Swapping out to BS aborted");	
		}
		else
		{
			write_bs((char *)((*frame_number)<<12), store, pageth);		
		}
	}
	if(debug) kprintf("\n Swapped Out Frame no # %d , was_dirty = %d ",*frame_number, is_dirty);
 free_frm(*frame_number, pid);
	if(pr_debug) kprintf("\n Swapped Out Frame no # %d",*frame_number);

 return OK;
}



/*
int get_AGING_policy_victim(int * frame_number, int * is_dirty, unsigned long * vpno1,int *pid1)
{  // Sorry, but I had to duplicate the code.
	int ct=0, p;
	unsigned long pdbr, pd_off, pt_off;
	unsigned char min_ctr= 0xff;
	if(debug) kprintf("\nmin_Ctr %d", min_ctr);
	unsigned long *vpno , *pid;
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
			SET_BIT(frm_tab[p].ctr, 7);
      if (debug) kprintf("\nfrm_tab[p].ctr = %d", frm_tab[p].ctr);
			(*pte).pte.pt_acc = 0;  // Set the accessed bit to 0
		}
		if(frm_tab[p].ctr < min_ctr)
		{
			min_ctr = frm_tab[p].ctr;
			(*frame_number) = (p + ENTRIES_PER_PAGE);
			(*is_dirty) = (*pte).pte.pt_dirty;
			(*vpno1) = frm_tab[p].fr_vpno;
			(*pid1) = frm_tab[p].fr_pid;
		}
		p = frm_tab[p].next;
		if(p == sc_head)
			break;
	}
	if(ct > NFRAMES)
	{
		kprintf("The queue has not been set up properly ct = %d  NFRAMES= %d", ct, NFRAMES);
		return SYSERR;
	}
	// Move the sc_head one step forward to give it the illusion of removing and inserting the frame.
	sc_head = frm_tab[sc_head].next;
	frm_tab[(*frame_number-ENTRIES_PER_PAGE)].ctr =0;
 	if(debug) kprintf("\n frame_number %d, is_dirty= %d, vpno= 0x%08x, pid= %d min_ctr = %d",*frame_number, *is_dirty, *vpno1, *pid1, min_ctr);

	return OK;
}

*/


int insert_bs_fr_tab_info(bsd_t bs_id, int pageth, int frame_no)
{
	int i = 0;
	frame_no -= ENTRIES_PER_PAGE;
	if(frm_tab[frame_no].fr_status == FRM_UNMAPPED)
	{
		if(debug) kprintf("\nWARNING: Putting BS-Frame map fata in UNMAPPED Frame No# %d",frame_no);
	}
	frm_tab[frame_no].bs_id = bs_id;
	frm_tab[frame_no].pageth = pageth;
	return OK;

}

int remove_bs_fr_tab_info(bsd_t bs_id, int pageth, int frame_no)
{
	frame_no -= ENTRIES_PER_PAGE;
	if(debug) kprintf("\n remove_bs_fr_tab_info bs_id = %d, pageth = %d, frame no= %d",bs_id, pageth, frame_no);
	frm_tab[frame_no].bs_id = -1;
	frm_tab[frame_no].pageth = -1;
	return OK;
}

int find_bs_fr_tab_info(bsd_t bs_id, int pageth)
{// Returns the frame number mapped to bs_id and pageth.
	int frame_no = 512;
	for(frame_no=512; frame_no < ENTRIES_PER_PAGE; frame_no++)
	{
		if(frm_tab[frame_no].bs_id == bs_id && frm_tab[frame_no].pageth == pageth)
			return frame_no+ENTRIES_PER_PAGE;
	}
	return SYSERR;
}

int get_bs_offset(int frame_no, bsd_t *bs_id, int *pageth)
{	frame_no -= ENTRIES_PER_PAGE;
	if(frm_tab[frame_no].bs_id != -1 && frm_tab[frame_no].pageth != -1)
	{
		*bs_id = frm_tab[frame_no].bs_id ;
		*pageth = frm_tab[frame_no].pageth;
		if(debug) kprintf("\n get_bs_offset bs_id = %d, pageth = %d, frame no= %d",*bs_id, *pageth, frame_no);
		return OK;
	}
	return SYSERR;
}

SYSCALL update_inverted_pt_entry(int pid, int frame_no, int status, int vpno, int type, int shared)
{
	/* frame_no should be between 1024 to 2048 */
	if (frame_no < ENTRIES_PER_PAGE || frame_no >= ENTRIES_PER_PAGE*2)
		return SYSERR;
	int i, j;
	frame_no -= ENTRIES_PER_PAGE;
	frm_tab[frame_no].fr_status = status;
	for(i=0; i < MAX_PROCESS_PER_BS ; i++)
	{
		if(frm_tab[frame_no].pr_map[i].bs_pid == pid)
		{	frm_tab[frame_no].pr_map[i].bs_vpno = vpno;
			break;
		}
	}
	if(i >= MAX_PROCESS_PER_BS)
	{
		for(j=0; j< MAX_PROCESS_PER_BS; j++)
		{
			if(frm_tab[frame_no].pr_map[j].bs_pid == -1)
			{	
				frm_tab[frame_no].pr_map[j].bs_pid = pid;
				frm_tab[frame_no].pr_map[j].bs_vpno = vpno;
				break;
			}
		}
		if(j >= MAX_PROCESS_PER_BS)
		{
			kprintf("\nFailed to update inverted page table for frame_no # %d, i= %d, j=%d",frame_no, i, j);
			return SYSERR;
		}
	}
	frm_tab[frame_no].fr_refcnt++;
	frm_tab[frame_no].fr_type = type;
	frm_tab[frame_no].fr_dirty = 0;
	frm_tab[frame_no].shared = shared;
	if(debug) kprintf("\nUPDATED Inv PT entry # %d, status= %d, vpno= 0x%x, type = %d",frame_no, status, vpno, type);
	return OK;
}
