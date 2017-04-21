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

#define MAX_PROCESS_PER_BS 5
#define GET_PD_OFFSET(vaddr) ((vaddr >>22) << 2)
#define GET_PT_OFFSET(vaddr) (((vaddr << 10) >> 22) << 2)
#define GET_PG_OFFSET(vaddr) ((vaddr << 20) >> 20)

#define NBPG		4096	/* number of bytes per page	*/
#define ENTRIES_PER_PAGE 		1024
#define FRAME0		1536	/* zero-th frame. I have reserved frames [1024 - 1535] for Page Directories
						and page table entries. The four global page tables[1024-1027], Null process's 
						page directory[1028] and the rest [1029-1078] for remaining (49) process' page directories
						Frame 1078-1535 is for other page tables.*/
#define NFRAMES 	5	/* number of available frames. NEVER SET THIS TO MORE THAN 512.*/

#define BSM_UNMAPPED	0
#define BSM_MAPPED	1
#define FRM_UNMAPPED	0
#define FRM_MAPPED	1

#define FR_PAGE		0
#define FR_TBL		1
#define FR_DIR		2

#define SC 3
#define AGING 4

#define BACKING_STORE_BASE	0x00800000
#define BACKING_STORE_UNIT_SIZE 0x00100000


typedef unsigned int	 bsd_t;

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
  map_struct pr_map[MAX_PROCESS_PER_BS];
  int fr_refcnt;			/* reference count		*/
  int fr_type;				/* FR_DIR, FR_TBL, FR_PAGE	*/
  int fr_dirty;				/* If the frame in the memory has been written into */
  int next;					/* For use in SC queue */
  unsigned char ctr:8;				/* For use in AGING PR policy */
  int shared; 				/* If this frame is shared  */
  bsd_t bs_id;				
  int pageth;				/* Bs_id and pageth give the page this frame is mapped to */
}fr_map_t;

/* To keep track of which page from a BS is stored in which frame */
typedef struct
{
	int 	valid;	/* Is this entry valid  */
	bsd_t 	bs_id;		/* BS ID */
	int 	pageth;		/* the page offset in bs_id  */
	int 	fr_no;    	/* fr_no shall range from 1024 to 2048  */
}bs_fr_map;

extern int 		pr_debug;
extern int 		page_replace_policy;
extern int 		sc_head;  // Second Change PR policy queue head
extern int 		fifo_head;  // AGING policy head
extern bs_map_t bsm_tab[];
extern fr_map_t frm_tab[];
//extern bs_fr_map bs_fr_tab[];
extern unsigned long pferrcode;
extern unsigned long gpt_base_address[] ;  // Keeps the base address of 4 global page tables.
// Useful while initializing a process' page directory.

/* Prototypes for required API calls */

/* paging.c */
SYSCALL initialize_4_global_page_tables(int frame_no);
pd_t* initialize_page_directory(int frame_no);
void convert_vaddr_to_paddr(unsigned long pdbr, unsigned long cr2);


/* frame.c */
SYSCALL init_frm();
SYSCALL get_frame_for_PD(int pid, int * frame_number);
SYSCALL get_frame_for_PT(int *frame_number);
SYSCALL clean_up_inverted_page_table(int pid);
SYSCALL free_frm(int frame_no, int pid);
SYSCALL get_victim_frame(int * frame_number);
SYSCALL get_frm(int* frame_number);
SYSCALL insert_bs_fr_tab_info(bsd_t bs_id, int pageth, int fr_no);
SYSCALL remove_bs_fr_tab_info(bsd_t bs_id, int pageth, int fr_no);
SYSCALL find_bs_fr_tab_info(bsd_t bs_id, int pageth);
SYSCALL update_inverted_pt_entry(int pid, int frame_no, int status, int vpno, int type,int shared);
SYSCALL get_bs_offset(int frame_no, bsd_t *bs_id, int *pageth);


/* get_bs.c */
SYSCALL get_bs(bsd_t, unsigned int npages);
SYSCALL reserve_bs(int pid, bsd_t bs_id, unsigned int npages, int shared);

/* release_bs.c */
SYSCALL release_bs(bsd_t);
SYSCALL __release_bs__(int pid, bsd_t bs_id);

/* given calls for dealing with backing store */
SYSCALL read_bs(char *, bsd_t, int);
SYSCALL write_bs(char *, bsd_t, int);

/* bsm.c */
SYSCALL init_bsm();
SYSCALL get_bsm(bsd_t* bsm_id);
SYSCALL free_bsm(int pid);
SYSCALL bsm_map(int pid, int vpno, bsd_t bs_id, int npages);
SYSCALL bsm_unmap(int pid, int vpno, int flag);
SYSCALL is_bsm_available(bsd_t bsm_id, int pid, int * bs_shared);
SYSCALL bsm_lookup(int pid, unsigned long vaddr, int* store, int* pageth);

/* pfintr.S  Interrupt hander for INT 14 page fault */
void pfintr();

/* pfint.c */
SYSCALL pfint();
SYSCALL dummy_pfint(unsigned long cr2);
SYSCALL handle_shared_memory_usecase(bsd_t store, int pageth, int * frame_no);


/* policy.c */
SYSCALL srpolicy(int policy);
SYSCALL grpolicy();
SYSCALL get_AGING_policy_victim(int * frame_number, int * is_dirty, unsigned long * vpno1,int *pid1);
SYSCALL get_SC_policy_victim(int * frame_number, int * is_dirty, unsigned long * vpno,int *pid);

SYSCALL insert_into_sc_queue(int frame_no);
SYSCALL remove_from_sc_queue(int frame_no);
SYSCALL test_sc_queue();
void print_sc_queue();


SYSCALL insert_into_fifo_queue(int frame_no);
SYSCALL is_present_in_fifo_queue(int frame_no);
SYSCALL remove_from_fifo_queue(int frame_no);
SYSCALL test_fifo_queue();
void print_fifo_queue();
void print_queue();



/* xm.c */
SYSCALL xmmap(int virtpage, bsd_t source, int npages);
SYSCALL xmunmap(int virtpage);

#endif
