#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/* Initialize the 4 page tables for the */
SYSCALL initialize_4_global_page_tables(int frame_no)
{
	/*
	Call me with frame_no value as 1024. If you send something else then 
	I will make it 1024 anyways.
	*/
	frame_no = 1024;
	pt_t * pte_address;
	int i,j;
	STATWORD ps;
	disable(ps);
	for(i=0; i< 4; i++)
	{
		//TODO: Update inverted page table entries.
		pte_address = (pt_t*)(frame_no * NBPG);
		for(j=0; j< ENTRIES_PER_PAGE; j++)
		{
			pte_address[j].dummy = 0;
			pte_address[j].pte.pt_base = (i*ENTRIES_PER_PAGE) + j;
			pte_address[j].pte.pt_pres = 1;
			pte_address[j].pte.pt_write = 1;
		}
		if (debug) kprintf("\npte_address = 0x%x frame_no= %d",pte_address, frame_no);
		gpt_base_address[i] = (unsigned long)pte_address;
		frame_no++;
	}
	restore(ps);
	return OK;
}


void convert_vaddr_to_paddr(unsigned long pdbr, unsigned long vaddr)
{
	/* This procedure is not supposed to be called. 
	It's just a sanity check if 
	the paging mechanism is able to translate the first 16 MB of virtual address
	to physical address or not.
	This procedure does the calculation and then verifies whether the first 16MB 
	of virtual address space maps to the same value in physical address space.
	*/
	unsigned long paddr;
	//kprintf("\n\n\npdbr = 0x%x ",pdbr);
	pdbr = (pdbr >> 12) << 12;
	//kprintf("\npdbr = 0x%x ",pdbr);
	unsigned int pd_offset, pt_offset, pg_offset;
	pd_offset = GET_PD_OFFSET(vaddr);
	pt_offset = GET_PT_OFFSET(vaddr);
	pg_offset = GET_PG_OFFSET(vaddr);
	//kprintf("\n pd_offset = 0x%x, pt_offset 0x%x, pg_offset 0x%x", pd_offset, pt_offset, pg_offset);

	pd_t pde = *((pd_t*)(pdbr + pd_offset));
	//kprintf("\npde.pd_base = 0x%x",pde.pd_base);
	if (pde.pde.pd_pres == 0 || pde.pde.pd_write == 0)
		kprintf("\n Stage 1 error: address = 0x%x , value 0x%x\n",&pde, pde.dummy);
	
	pt_t pte = *((pt_t*)((pde.pde.pd_base << 12) + pt_offset ));
	//kprintf("\npte.pt_base = 0x%x",pte.pt_base<<12);
	if (pte.pte.pt_pres == 0 || pte.pte.pt_write == 0)
		kprintf("\n Stage 2 error: address = 0x%x , value 0x%x\n",&pte, pte.dummy);
	paddr = (pte.pte.pt_base << 12) + pg_offset;
	//kprintf("\nVaddr = 0x%x  paddr = 0x%x",vaddr, paddr);
	if(vaddr == paddr)
		kprintf(".");
	else
		kprintf("x");
}


pd_t * initialize_page_directory(int frame_no)
{
	/*
	Initializes a page directory in frame_no. First 4 entries contain the address
	for the 4 global page tables. Rest are initialized to 0.
	
	Returns (frame_no << 12)
	*/
	int i;
	STATWORD ps;
	disable(ps);
	pd_t * addr = (pd_t*)(frame_no << 12);  // Equivalent to multiplying by 4096.
	if(debug) kprintf("\npage directory base address = 0x%x", addr);
	for(i=0; i < ENTRIES_PER_PAGE; i++)
		addr[i].dummy = 0;

	// Write the first 4 entries to correct values.
	if(debug) kprintf("\nfirst 5 entries in page directory");
	for(i=0; i<4; i++)
	{
		addr[i].pde.pd_pres = 1;
		addr[i].pde.pd_write = 1;
		addr[i].pde.pd_base = (gpt_base_address[i]) >> 12;
		if(debug) kprintf("\naddress= 0x%x content= 0x%x",addr+i,addr[i].dummy);
	}
	restore(ps);
	return addr;
}

SYSCALL update_inverted_pt_entry(int frame_no, int status, int vpno, int type)
{
	/* frame_no should be between 1024 to 2048 */
	if (frame_no < ENTRIES_PER_PAGE || frame_no >= ENTRIES_PER_PAGE*2)
		return SYSERR;
	int i, j;
	frame_no -= ENTRIES_PER_PAGE;
	frm_tab[frame_no].fr_status = status;
	for(i=0; i< ENTRIES_PER_PAGE; i++)
	{
		if(frm_tab[frame_no].pr_map[i].bs_pid == currpid)
		{	frm_tab[frame_no].pr_map[i].bs_vpno = vpno;
			break;
		}
	}
	if(i >= ENTRIES_PER_PAGE)
	{
		for(j=0; j< ENTRIES_PER_PAGE; j++)
		{
			if(frm_tab[frame_no].pr_map[j].bs_pid == -1)
			{	
				frm_tab[frame_no].pr_map[j].bs_pid = currpid;
				frm_tab[frame_no].pr_map[j].bs_vpno = vpno;
				break;
			}
		}
		if(j >= ENTRIES_PER_PAGE)
		{
			kprintf("\nFailed to update inverted page table for frame_no # %d",frame_no);
			return SYSERR;
		}
	}
	frm_tab[frame_no].fr_refcnt++;
	frm_tab[frame_no].fr_type = type;
	frm_tab[frame_no].fr_dirty = 0;
	if(debug) kprintf("\nUPDATED Inv PT entry # %d, status= %d, vpno= 0x%x, type = %d",frame_no, status, vpno, type);
	return OK;
}
