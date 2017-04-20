/* policy.c = srpolicy*/

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * srpolicy - set page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL srpolicy(int policy)
{
	if(sc_head == -1 && fifo_head == -1)
	{
	  pr_debug = TRUE;
	  switch(policy)
	  { // policy wise code initialization
		case AGING:
				fifo_head = -1;
				break;
		default:  /* Fall Through */
		case SC:
				sc_head = -1;
	  }
	  page_replace_policy = policy;
	  return OK;
	}
	else
	{
	kprintf("\n WARNING: srpolicy(AGING), if called, will be the first statement in the program");
	}
 }

/*-------------------------------------------------------------------------
 * grpolicy - get page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL grpolicy()
{
  return page_replace_policy;
}


int insert_into_sc_queue(int frame_no)
{
	STATWORD ps;
	disable(ps);
	if (frame_no < 512 || frame_no > 512 + NFRAMES)
	{
		kprintf("\n Invalid Frame # %d Range of value is 512 to %d", frame_no, 512 + NFRAMES);
		restore(ps);
		return SYSERR;
	}
	// If queue is empty.
	if (sc_head == -1)
	{
		sc_head = frame_no;
		frm_tab[sc_head].next = sc_head;
		restore(ps); return OK;
	}

	int q = sc_head;
	int p = frm_tab[sc_head].next;
	int ct = 0;
	while(p != sc_head && p != frame_no && ct <= 512)
	{ 	ct++;
		q = p; 
		p = frm_tab[p].next;
	}
	if(ct > 512 || p == frame_no)
	{
		kprintf("\n SC queue is bad. sc_head = %d, ct= %d, p = %d, frame_no= %d",sc_head, ct, p, frame_no);
		restore(ps); return SYSERR;
	}
	// Insert
	frm_tab[q].next = frame_no;
	frm_tab[frame_no].next = p;
	restore(ps);
	return OK;
}


int insert_into_fifo_queue(int frame_no)
{
	STATWORD ps;
	disable(ps);
	if (frame_no < 512 || frame_no > 512 + NFRAMES)
	{
		kprintf("\n Invalid Frame # %d Range of value is 512 to %d", frame_no, 512 + NFRAMES);
		restore(ps);
		return SYSERR;
	}
	// queue is empty
	if(fifo_head == -1)
	{
		fifo_head = frame_no;
		frm_tab[fifo_head].next = -1;
		restore(ps); return OK;
	}
	int p = fifo_head;
	int ct = 0;
	while(frm_tab[p].next != -1 && p != frame_no && ct <= 512)
	{
		p = frm_tab[p].next;
		ct++;
	}
	if(ct > 512 || p == frame_no)
	{
		kprintf("\n FIFO queue is bad. fifo_head = %d, ct= %d, p = %d, frame_no= %d",fifo_head, ct, p, frame_no);
		restore(ps); return SYSERR;
	}
	frm_tab[p].next = frame_no;
	frm_tab[frame_no].next = -1;
	restore(ps);
	return OK;
}


int is_present_in_sc_queue(int frame_no)
{
	// Returns TRUE if frame_no is found in queue otherwise FALSE.
	STATWORD ps;
	disable(ps);
	if (frame_no < 512 || frame_no > 512 + NFRAMES)
	{
		kprintf("\n Invalid Frame # %d Range of value is 512 to %d", frame_no, 512 + NFRAMES);
		restore(ps);
		return SYSERR;
	}
	// EMpty queue
	if (sc_head == -1)
	{ restore(ps); return FALSE;}

	// HEad node is the one needed.
	if (frame_no == sc_head)
	{ restore(ps); return TRUE;}

	int p = frm_tab[sc_head].next;
	int ct=0;
	while(p != sc_head && p != frame_no && ct <= 512)
	{
		p = frm_tab[p].next;
		ct++;
	}
	if(ct > 512)
	{
		kprintf("\nIterating SC queue goes infinite");
		restore(ps);
		return SYSERR;
	}
	if (p == sc_head)
	{
		kprintf("\nframe no # %d not in sc queue. sc_head = %d ", frame_no, sc_head);
		restore(ps); return SYSERR;
	}
	restore(ps);
	return TRUE;
}

int is_present_in_fifo_queue(int frame_no)
{
	// Returns TRUE if frame_no is found in queue otherwise FALSE.
	STATWORD ps;
	disable(ps);
	if (frame_no < 512 || frame_no > 512 + NFRAMES)
	{
		kprintf("\n Invalid Frame # %d Range of value is 512 to %d", frame_no, 512 + NFRAMES);
		restore(ps);
		return SYSERR;
	}
	// EMpty queue
	if(fifo_head == -1)
	{
		restore(ps); return FALSE;
	}
	
	int p = fifo_head;
	int ct = 0;
	while(p != -1 && p != frame_no && ct<=512)
	{
		p = frm_tab[p].next;
		ct++;
	}
	if(ct > 512)
	{
		kprintf("\nIterating FIFO queue goes infinite");
		restore(ps);
		return SYSERR;
	}
	if(p == -1)
	{
		kprintf("\nframe no # %d not in FIFO queue. fifo_head = %d ", frame_no, fifo_head);
		restore(ps); return SYSERR;
	}
	restore(ps);
	return TRUE;
}


int remove_from_sc_queue(int frame_no)
{
	if (frame_no < 512 || frame_no > 512 + NFRAMES)
	{
		kprintf("\n Invalid Frame # %d Range of value is 512 to %d", frame_no, 512 + NFRAMES);
		return SYSERR;
	}
	if(debug) kprintf("\n remove_from_sc_queue called for frame_no# %d",frame_no);
	int present = is_present_in_sc_queue(frame_no);
	if (present == FALSE)
	{
		return SYSERR;
	}
	STATWORD ps;
	disable(ps);
	
	// At this point frame_no is in the queue
	if(frm_tab[sc_head].next == sc_head)
	{	// There is only one node in queue and that is to be removed.
		frm_tab[sc_head].next = -1;
		sc_head = -1;
		restore(ps);
		return -1;  // This will indicate that the queue is now empty
	}
	
	// At this point there are more than 1 node in the queue.
	int q = sc_head;
	int p = frm_tab[sc_head].next;
	int ct=0;
	while(p != frame_no && ct <= 512)
	{
		q = p;
		p = frm_tab[p].next;
		ct++;
	}
	if(ct > 512)
	{
		kprintf("\nIterating SC queue goes infinite");
		restore(ps);
		return SYSERR;
	} 
	// Take care if the node to be removed is the head node.
	frm_tab[q].next = frm_tab[p].next;
	frm_tab[p].next = -1;
	sc_head = frm_tab[q].next;
	restore(ps);
	return frm_tab[q].next;
}

int remove_from_fifo_queue(int frame_no)
{
	if (frame_no < 512 || frame_no > 512 + NFRAMES)
	{
		kprintf("\n Invalid Frame # %d Range of value is 512 to %d", frame_no, 512 + NFRAMES);
		return SYSERR;
	}
	if(debug) kprintf("\n remove_from_fifo_queue called for frame_no# %d",frame_no);
	int present = is_present_in_fifo_queue(frame_no);
	if(present == FALSE)
		return SYSERR;
	STATWORD ps;
	disable(ps);
	// At this point frame_no is in the queue
	if(frm_tab[fifo_head].next == -1)
	{	// There is only one node in queue and that is to be removed.
		frm_tab[fifo_head].next = -1;
		fifo_head = -1;
		restore(ps);
		return -1;   // This will indicate that the queue is now empty
	}
	
	// At this point there are more than 1 node in the queue.
	if(frame_no == fifo_head)
	{// Check if removing the head node.
	   fifo_head = frm_tab[fifo_head].next;
	   frm_tab[frame_no].next = -1;
	   restore(ps);
	   return OK;
	}
	
	int p = fifo_head;
	int ct=0;
	while(p!= -1 && frm_tab[p].next != frame_no && ct <= 512)
	{
		p = frm_tab[p].next;
		ct++;
	}
	if(ct > 512 || p == -1)
	{
		kprintf("\nIterating FIFO queue goes infinite. CHECK this p= %d",p);
		restore(ps);
		return SYSERR;
	}
	frm_tab[p].next = frm_tab[frame_no].next;
	frm_tab[frame_no].next = -1;
	restore(ps);
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
	int ct=0, i=0, temp_pid, accessed;
	unsigned long pdbr, temp_vpno, pd_off, pt_off;
	
	// The ptes variable is used to keep track of which 
	// PTE values need to be invalidated in case the frame shall need to be removed.
	pt_t  *pte_s[MAX_PROCESS_PER_BS];
	int victim_dirty = FALSE;
	int count=0;
	pt_t *pte;
	for(; ct <= NFRAMES; ct++)
	{	
		if(sc_head == -1)
		{
			kprintf("\nget_SC_policy_victim: SC queue is not set sc_head %d",sc_head);
			return SYSERR;
		}
		accessed = FALSE;
		count = 0;
		for(i=0; i<MAX_PROCESS_PER_BS; i++)
		{
			temp_pid = frm_tab[sc_head].pr_map[i].bs_pid;
			if(temp_pid == -1)
				continue;
			temp_vpno = frm_tab[sc_head].pr_map[i].bs_vpno;
			pdbr = ((proctab[temp_pid].pdbr)>>12)<<12;  // Clear the 12 LSB bits.
			pd_off = ((temp_vpno) <<12) >> 22;  // Double check this. vpno shall have only 20 LSB bits.
			pt_off = ((temp_vpno) << 22) >> 22; 
			if(debug) kprintf("\nDouble Check these values: pdbr = 0x%x, vpno = 0x%08x pd_off 0x%x pt_off 0x%x ",pdbr,temp_vpno, pd_off,pt_off);
			pd_off = pd_off << 2;  // Multiply by 4
			pt_off = pt_off << 2;
			pd_t *pde = (pd_t*)(pdbr + pd_off);
			if ((*pde).pde.pd_pres == 0)
			{kprintf("\nSTAGE 1: We are in deep trouble."); return SYSERR;}
			pte = (pt_t*)(((*pde).pde.pd_base << 12) + pt_off);
			if ((*pte).pte.pt_pres == 0)
			{kprintf("\nSTAGE 2: We are in deep trouble."); return SYSERR;}
			// If this frame was written by any process then mark it as dirty. Used when a processes is killed 
			// and we need to figure out if this should be written into memory
			frm_tab[sc_head].fr_dirty = frm_tab[sc_head].fr_dirty || (*pte).pte.pt_dirty;
			if((*pte).pte.pt_acc == 1)
			{  // At least one process has accessed this frame. THis will get another chance.
				accessed = TRUE;
				if(debug) kprintf("\nThe page has been accessed");
				(*pte).pte.pt_acc = 0;  // Set the accessed bit to 0 and move on.
			}
			else
			{  // This might not get a second chance. Let's store the pte so that we can
				// invalidate the entry in case this frame is swapped out.
				pte_s[count] = pte;
				victim_dirty = (*pte).pte.pt_dirty || victim_dirty;
				count++;
				*pid = temp_pid;
				*vpno = temp_vpno;
			
			}
		}
		if(accessed == FALSE)
		{   // Found the victim. Current frame is the victim.
			*frame_number = sc_head + ENTRIES_PER_PAGE;
			*is_dirty = victim_dirty;
			if(debug) kprintf("\n THE BITS SHOULD NOT BE ALL ZERO 0x%08x ",(*pte).dummy);

			for(i=0; i<count; i++)
				(*pte_s[i]).dummy = 0;
			if(debug) kprintf("\n CHECK IF THE BITS ARE ALL ZERO 0x%08x ",(*pte).dummy);
			// TODO: Decrease the ref_cnt from inverted page table. if it becomes zero then invalidate the PDE.
			// TODO: Invalidate the TLB entry.
			// Remove the frame from the queue.
			// See if we need to remove from the queue. Currently, I think we don't.
			sc_head = frm_tab[sc_head].next;
			return OK;
		}
		sc_head = frm_tab[sc_head].next;
		
	}
	// If the code reaches here, this means the circular queue is not properly set or something is smelling.
	// A good day to die hard.
	return SYSERR;

}

int get_AGING_policy_victim(int * frame_number, int * is_dirty, unsigned long * vpno1,int *pid1)
{
}

void print_sc_queue()
{
	if(sc_head == -1)
	{kprintf("\nEmpty SC queue"); return;}
	kprintf("\n Queue Head= %d \n Frame No\tBS ID\tOffset\tNext", sc_head+ENTRIES_PER_PAGE);
	int p = frm_tab[sc_head].next;
	int ct=0;
	kprintf("\n %9d\t%5d\t%6d\t%4d", sc_head+ENTRIES_PER_PAGE, frm_tab[sc_head].bs_id, frm_tab[sc_head].pageth, frm_tab[sc_head].next+ENTRIES_PER_PAGE);

	while(p != sc_head && ct <= 512)
	{
		kprintf("\n %9d\t%5d\t%6d\t%4d", p+ENTRIES_PER_PAGE, frm_tab[p].bs_id, frm_tab[p].pageth, frm_tab[p].next+ENTRIES_PER_PAGE);
		p = frm_tab[p].next;
		ct++;
	}
	if(ct > 512)
		kprintf("\nSC Queue wrongly formed");
}


void print_fifo_queue()
{
	if(fifo_head == -1)
	{kprintf("\nEmpty FIFO queue"); return;}
	kprintf("\n Queue Head= %d \n Frame No\tBS ID\tOffset\tNext", fifo_head+ENTRIES_PER_PAGE);
	int p = fifo_head;
	int ct = 0;
	while(p != -1 && ct < 512)
	{
		kprintf("\n %9d\t%5d\t%6d\t%4d", p+ENTRIES_PER_PAGE, frm_tab[p].bs_id, frm_tab[p].pageth, frm_tab[p].next+ENTRIES_PER_PAGE);
		p = frm_tab[p].next;
		ct++;		
	}
	if(ct > 512)
		kprintf("\nFIFO Queue wrongly formed");

}

int test_fifo_queue()
{
	kprintf("\n FIFO HEAD = %d", fifo_head);
	print_fifo_queue();
	insert_into_fifo_queue(512);
	print_fifo_queue();

	insert_into_fifo_queue(513);
	print_fifo_queue();

	insert_into_fifo_queue(514);
	print_fifo_queue();
	insert_into_fifo_queue(515);
	print_fifo_queue();
	remove_from_fifo_queue(514);
	print_fifo_queue();
	remove_from_fifo_queue(512);	
	print_fifo_queue();
	remove_from_fifo_queue(513);
	print_fifo_queue();
	remove_from_fifo_queue(515);
	print_fifo_queue();
	remove_from_fifo_queue(113);
	print_fifo_queue();
	//print_sc_queue();
	return OK;

	
}
int test_sc_queue()
{
	kprintf("\nsc_head = %d", sc_head);
	print_sc_queue();
	insert_into_sc_queue(12);
	insert_into_sc_queue(514);
	insert_into_sc_queue(518);
	insert_into_sc_queue(513);
	print_sc_queue();
	remove_from_sc_queue(12);
	kprintf("\n %d",remove_from_sc_queue(514));
	kprintf("\n %d",remove_from_sc_queue(518));
	kprintf("\n %d",remove_from_sc_queue(513));
	kprintf("\n %d",remove_from_sc_queue(113));
	print_sc_queue();
	//print_sc_queue();
	return OK;
}