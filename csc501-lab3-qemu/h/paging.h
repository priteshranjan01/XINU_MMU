/* paging.h */

/*
From Intel reference manual volume 3
Paging behavior is controlled by the following control bits:
The WP and PG flags in control register CR0 (bit 16 and bit 31, respectively).
The PSE, PAE, PGE, PCIDE, SMEP, SMAP, and PKE flags in control register CR4 (bit 4, bit 5, bit 7, bit 17, bit 20,
bit 21, and bit 22, respectively).
The LME and NXE flags in the IA32_EFER MSR (bit 8 and bit 11, respectively).
The AC flag in the EFLAGS register (bit 18).

Software enables paging by using the MOV to CR0 instruction to set CR0.PG. Before doing so, software should
ensure that control register CR3 (PDBR) contains the physical address of the first paging structure that the processor will
use for linear-address translation (see section 4.2)

paging is enabled if CR0.PG = 1 and CR0.PE=1
If CR0.PG=1 and CR4.PAE=0 then 32 bit paging. THIS IS WHAT we are studying.
32-bit paging uses CR0.WP, CR4.PSE, CR4.PGE, CR4.SMEP, and CR4.SMAP as described in Section 4.1.3

*/
#ifndef _PAGING_H_
#define _PAGING_H_
typedef unsigned int	 bsd_t;
#define MAX_PROCESS_PER_BS 5
/* Structure for a page directory entry */

typedef struct {

  unsigned int pd_pres	: 1;		/* page table present?		*/
  unsigned int pd_write : 1;		/* page is writable?		*/
  unsigned int pd_user	: 1;		/* is use level protection?	*/
  unsigned int pd_pwt	: 1;		/* write through cachine for pt?*/
  unsigned int pd_pcd	: 1;		/* cache disable for this pt?	*/
  unsigned int pd_acc	: 1;		/* page table was accessed?	*/
  unsigned int pd_mbz	: 1;		/* must be zero			*/
  unsigned int pd_fmb	: 1;		/* four MB pages?		*/
  unsigned int pd_global: 1;		/* global (ignored)		*/
  unsigned int pd_avail : 3;		/* for programmer's use		*/
  unsigned int pd_base	: 20;		/* location of page table?	*/
} pd_struct;

typedef union {
	pd_struct pde;			/* page directory entry */
	unsigned long dummy;	/* 32 bit value for one line initialization */
}pd_t;
/* Structure for a page table entry */

typedef struct {

  unsigned int pt_pres	: 1;		/* page is present?		*/
  unsigned int pt_write : 1;		/* page is writable?		*/
  unsigned int pt_user	: 1;		/* is use level protection?	*/
  unsigned int pt_pwt	: 1;		/* write through for this page? */
  unsigned int pt_pcd	: 1;		/* cache disable for this page? */
  unsigned int pt_acc	: 1;		/* page was accessed?		*/
  unsigned int pt_dirty : 1;		/* page was written?		*/
  unsigned int pt_mbz	: 1;		/* must be zero			*/
  unsigned int pt_global: 1;		/* should be zero in 586	*/
  unsigned int pt_avail : 3;		/* for programmer's use		*/
  unsigned int pt_base	: 20;		/* location of page?		*/
} pt_struct;

typedef union {
	pt_struct pte;		/* page table entry */
	unsigned long dummy;  /* 32 bit value for one line initialization */
}pt_t;

typedef struct{
  unsigned int pg_offset : 12;		/* page offset			*/
  unsigned int pt_offset : 10;		/* page table offset		*/
  unsigned int pd_offset : 10;		/* page directory offset	*/
} virt_addr_t;

typedef struct{
	int bs_pid;				/* process id using this slot   */
	int bs_vpno;			/* starting virtual page number */
}map_struct;


typedef struct{
  int bs_status;			/* MAPPED or UNMAPPED		*/
  map_struct pr_map[MAX_PROCESS_PER_BS];	/* In case a BS is mapped by multiple processes.
				Note: By design simultaneously only MAX_PROCESS_PER_BS processes can map to one BS*/
  int bs_npages;			/* number of pages in the store */
  int bs_sem;				/* semaphore mechanism ?	*/
  int shared;				/* whether this is shared by other processes */
} bs_map_t;


typedef struct{
  int fr_status;			/* MAPPED or UNMAPPED		*/
  int fr_pid;				/* process id using this frame  */
  int fr_vpno;				/* corresponding virtual page no*/
  int fr_refcnt;			/* reference count		*/
  int fr_type;				/* FR_DIR, FR_TBL, FR_PAGE	*/
  int fr_dirty;				/* If the frame in the memory has been written into */
  int next;					/* For use in SC queue */
}fr_map_t;

extern bs_map_t bsm_tab[];
extern fr_map_t frm_tab[];
extern unsigned long gpt_base_address[] ;  // Keeps the base address of 4 global page tables.
// Useful while initializing a process' page directory.

/* Prototypes for required API calls */
SYSCALL xmmap(int, bsd_t, int);
SYSCALL xunmap(int);

SYSCALL initialize_4_global_page_tables(int frame_no);
pd_t* initialize_page_directory(int frame_no);


/* given calls for dealing with backing store */

int get_bs(bsd_t, unsigned int);
SYSCALL release_bs(bsd_t);
SYSCALL read_bs(char *, bsd_t, int);
SYSCALL write_bs(char *, bsd_t, int);
void pfintr();

#define GET_PD_OFFSET(vaddr) ((vaddr >>22) << 2)
#define GET_PT_OFFSET(vaddr) (((vaddr << 10) >> 22) << 2)
#define GET_PG_OFFSET(vaddr) ((vaddr << 20) >> 20)

#define NBPG		4096	/* number of bytes per page	*/
#define ENTRIES_PER_PAGE 		1024
#define FRAME0		1078	/* zero-th frame. I have reserved frames [1024 - 1079] for the four
						global page tables[1024-1027], Null process's page directory[1028] 
						and the rest [1029-1078] for remaining process' page directories
						This is acceptable since the PA3 description says that the PD's shall
						not be swapped out.*/
#define NFRAMES 	970	/* number of available frames. NEVER SET THIS TO MORE THAN 970.*/

#define BSM_UNMAPPED	0
#define BSM_MAPPED	1

SYSCALL init_bsm();
SYSCALL get_bsm(bsd_t* bsm_id);
int is_bsm_available(bsd_t bsm_id, int pid);
SYSCALL dummy_pfint(unsigned long cr2);

#define FRM_UNMAPPED	0
#define FRM_MAPPED	1

#define FR_PAGE		0
#define FR_TBL		1
#define FR_DIR		2

#define SC 3
#define AGING 4

#define BACKING_STORE_BASE	0x00800000
#define BACKING_STORE_UNIT_SIZE 0x00100000

#endif
