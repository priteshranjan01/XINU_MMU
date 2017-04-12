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
	for(i=0; i< 4; i++)
	{
		//TODO: Update inverted page table entries.
		pte_address = (pt_t*)(frame_no * 4096);
		for(j=0; j< ENTRIES_PER_PAGE; j++)
		{
			pte_address[j].pt_base = (i*ENTRIES_PER_PAGE) + j;
			pte_address[j].pt_pres = 1;
			pte_address[j].pt_write = 1;
			pte_address[j].pt_user = 0;
			pte_address[j].pt_pwt  = 0;
			pte_address[j].pt_pcd = 0;
			pte_address[j].pt_acc = 0;
			pte_address[j].pt_dirty = 0;
			pte_address[j].pt_mbz = 0;
			pte_address[j].pt_global = 0;
			pte_address[j].pt_avail = 0;
		}
		if (debug) kprintf("\npte_address = 0x%x frame_no= %d",pte_address, frame_no);
		gpt_base_address[i] = (unsigned long)pte_address;
		frame_no++;
	}
	return OK;
}


void convert_vaddr_to_paddr(unsigned long pdbr, unsigned long vaddr)
{
	unsigned long paddr;
	//kprintf("\n\n\npdbr = 0x%x ",pdbr);
	pdbr = (pdbr >> 12) << 12;
	//kprintf("\npdbr = 0x%x ",pdbr);
	unsigned long temp;
	unsigned int pd_offset, pt_offset, pg_offset;
	pd_offset = (vaddr >>22) << 2;
	pt_offset = ((vaddr << 10) >> 22) << 2;
	pg_offset = (vaddr << 20) >> 20;
	//kprintf("\n pd_offset = 0x%x, pt_offset 0x%x, pg_offset 0x%x", pd_offset, pt_offset, pg_offset);

	pd_t pde = *((pd_t*)(pdbr + pd_offset));
	//kprintf("\npde.pd_base = 0x%x",pde.pd_base);
	if (pde.pd_pres == 0 || pde.pd_write == 0)
	{
		kprintf("\n Stage 1 error: address = 0x%x , value 0x%x\n",&pde, pde);
	}
	pt_t pte = *((pt_t*)((pde.pd_base << 12) + pt_offset ));
	//kprintf("\npte.pt_base = 0x%x",pte.pt_base<<12);
	if (pte.pt_pres == 0 || pte.pt_write == 0)
	{
		kprintf("\n Stage 2 error: address = 0x%x , value 0x%x\n",&pte, pte);
	}
	paddr = (pte.pt_base << 12) + pg_offset;
	//kprintf("\nVaddr = 0x%x  paddr = 0x%x",vaddr, paddr);
	if(vaddr == paddr)
		kprintf(".");
	else
		kprintf("x");
	}

pd_t* initialize_page_directory(int frame_no)
{
	/*
	Initializes a page directory into the frame_no specified.
	This enters the first 4 entries to point to the 4 shared global page tables.
	Rest 1020 are initialized to zero.
	NOTE: Ensure that frame_no is empty. This function doesn't perform any checks.
	*/
	//TODO: Update inverted page table entries.

	int i;
	pd_t * addr = (pd_t*)(frame_no << 12);  // Equivalent to multiplying by 4096.
	if(debug) kprintf("\npage directory base address = 0x%x", addr);
	for(i=0; i< ENTRIES_PER_PAGE; i++)
	{	addr[i].pd_pres = 0;
		addr[i].pd_write = 0;
		addr[i].pd_user = 0;
		addr[i].pd_pwt = 0;
		addr[i].pd_pcd = 0;
		addr[i].pd_acc = 0;
		addr[i].pd_mbz = 0;
		addr[i].pd_fmb = 0;
		addr[i].pd_global = 0;
		addr[i].pd_avail = 0;
		addr[i].pd_base = 0;
	}
	if(debug) kprintf("\nNull process' first 5 entries in page directory");
	for(i=0; i<4; i++)
	{
		addr[i].pd_pres = 1;
		addr[i].pd_write = 1;
		addr[i].pd_base = (gpt_base_address[i]) >> 12;
		if(debug) kprintf("\naddress= 0x%x content= 0x%x",addr+i,addr[i]);
	}
	return addr;
}

