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


void convert_vaddr_to_paddr(unsigned long pdbr, unsigned long cr2)
{
	/* This procedure is not supposed to be called. 
	It's just a sanity check if 
	the paging mechanism is able to translate the first 16 MB of virtual address
	to physical address or not.
	This procedure does the calculation and then verifies whether the first 16MB 
	of virtual address space maps to the same value in physical address space.
	*/
	unsigned long paddr;
	unsigned int pd_off, pt_off, pg_off;
	int frame_no, i, status;
	pd_off = GET_PD_OFFSET(cr2);
	pt_off = GET_PT_OFFSET(cr2);
	pg_off = GET_PG_OFFSET(cr2);
	pdbr = (pdbr >> 12) << 12;
	pd_t * pde = (pd_t*)(pdbr + pd_off);

	if ((*pde).pde.pd_pres == 0)
		kprintf("\n Stage 1: Page fault");
	pt_t * pte = (pt_t*)((((*pde).pde.pd_base) << 12) + pt_off) ;
	
	if ((*pte).pte.pt_pres == 0)
		kprintf("\n Stage 2: Page fault");
	
	paddr = (((*pte).pte.pt_base) << 12) + pg_off;
	kprintf("\nVaddr = 0x%08x  paddr = 0x%08x",cr2, paddr);
	if(cr2 == paddr)
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

