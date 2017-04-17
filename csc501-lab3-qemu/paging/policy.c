/* policy.c = srpolicy*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * srpolicy - set page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL srpolicy(int policy)
{
	  pr_debug = TRUE;
	  switch(policy)
	  { // policy wise code initialization
		case AGING:
		default:  /* Fall Through */
		case SC:
			sc_head = -1;
	  }
	  page_replace_policy = policy;
	  
	  return OK;
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
	STATWORD PS;
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
	frm_tab[frame_no] = p;
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
	int ct;
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

int remove_from_sc_queue(int frame_no)
{
	if (frame_no < 512 || frame_no > 512 + NFRAMES)
	{
		kprintf("\n Invalid Frame # %d Range of value is 512 to %d", frame_no, 512 + NFRAMES);
		return SYSERR;
	}
	int present = is_present_in_sc_queue(frame_no);
	if (present == FALSE)
	{
		return SYSERR;
	}
	STATWORD PS;
	disable(ps);
	
	// At this point frame_no is in the queue
	if(frm_tab[sc_head].next == sc_head)
	{	// There is only one node in queue and that is to be removed.
		frm_tab[sc_head].next = -1;
		sc_head = -1;
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


void print_sc_queue()
{
	if(sc_head == -1)
	{kprintf("\nEmpty SC queue"); return;}
	kprintf("\n sc_queue = %d ", sc_head);
	int p = frm_tab[sc_head].next;
	int ct=0;
	while(p != sc_head && ct <= 512)
	{
		kprintf("\t %d ", p);
		p = frm_tab[p].next;
		ct++;
	}
	if(ct > 512)
		kprintf("\nSC Queue wrongly formed");
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
	print_sc_queue();
}