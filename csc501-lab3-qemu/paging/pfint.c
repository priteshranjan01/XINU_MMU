/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{
  //kprintf("To be implemented!\n");
  if(debug) kprintf("\npage fault happened");
  STATWORD ps;
  disable(ps);
  /*
  read CR2
  PD_offset PT_offset PG_offset
  Find the value at the PD_offset in Page directory.
  if present bit is Zero
		Get a free frame for the PT
		Initialize with 1024 blank PTE entries.
		Enter the relevant information in corresponding PDE.
		Mark the present bit as 1 in the corresponding PDE.
  vpno = 20 MSB bits of vaddr 
  BSM_LOOKUP to get BS_ID, pageth 
  frame_number = get_frame
  read_bs( .....)
  Update the relevant entry in the corresponding PT 
  Mark the PTE as present.
  */
  kprintf("\nPage fault error code 0x%x",pferrcode);
  if (GET_BIT(pferrcode, 0) == 0)
	  dummy_pfint(read_cr2());
  else
  {
	  kprintf("\n The process ought be killed. It raised a page-level protection violation");
  }
  restore(ps);
  return OK;
}

SYSCALL dummy_pfint(unsigned long cr2)
{
//	unsigned long cr2 = cr2;
	unsigned long pdbr;
	unsigned int pd_off, pt_off, pg_off;
	int frame_no, i;
	pd_off = GET_PD_OFFSET(cr2);
	pt_off = GET_PT_OFFSET(cr2);
	pg_off = GET_PG_OFFSET(cr2);
	unsigned int vpno = cr2 >> 12;
	pdbr = read_cr3();
	pdbr = (pdbr >> 12) << 12;
	if(debug) kprintf("\npdbr = 0x%x ,cr2 = 0x%x , pd_off 0x%x pt_off 0x%x pg_off 0x%x",pdbr,cr2,pd_off,pt_off,pg_off);
	pd_t * pde = (pd_t*)(pdbr + pd_off);
	kprintf("\npde.pd_base = 0x%x",(*pde).pde.pd_base);
	
	if ((*pde).pde.pd_pres == 0)
	{	// Page table itself is not present.
		if (get_frame_for_PT(&frame_no) != OK)
		{
			kprintf("\nHouston, we got a problem");
			kill(currpid);
		}
		if(debug) kprintf("\nNew Page table is to be created in frame # %d",frame_no);
		(*pde).dummy = 0;  // First clear to remove any garbage.
		(*pde).pde.pd_base = frame_no;
		(*pde).pde.pd_pres = 1;
		(*pde).pde.pd_write = 1;
		kprintf("\npde = 0x%x",(*pde).dummy);
		pt_t * pt_base_address = (pt_t*)(frame_no << 12);

		for(i=0; i<ENTRIES_PER_PAGE; i++)
			pt_base_address[i].dummy = 0;
		// Update inverted page table entry.
		update_inverted_pt_entry(frame_no, FRM_MAPPED, frame_no, FR_TBL);
	}
	int store, pageth;
	bsm_lookup(currpid, cr2, &store, &pageth);
	if(debug) kprintf("\ncurrpid %d, store %d, pageth %d",currpid, store, pageth);
	if (get_frm(&frame_no) != OK)
	{
		kprintf("\nHouston, we got another problem");
		kill(currpid);
	}
	if(debug) kprintf("\nNew Page is created in frame # %d",frame_no);	
	read_bs((char *)(frame_no<<12), store, pageth);
	pt_t * pte = (pt_t*)((((*pde).pde.pd_base) << 12) + pt_off) ;
	(*pte).dummy = 0;
	(*pte).pte.pt_pres = 1;
	(*pte).pte.pt_write = 1;
	(*pte).pte.pt_base = frame_no;
	// Update inverted page table entry 
	update_inverted_pt_entry(frame_no, FRM_MAPPED, vpno, FR_PAGE);
	if(debug) kprintf("\n pte value = 0x%x", (*pte).dummy);
}