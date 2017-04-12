/* i386.h - DELAY */

#define	NBPG		4096


#define	NID		48
#define	NGD		 8

#define	IRQBASE		32	/* base ivec for IRQ0			*/

/* Interrupt Descriptor		*/
/* See Intel Vol 3 Ch 5		*/
/* Figure 6-2 IDT Gate Descriptor */

struct idt {
	unsigned short	igd_loffset;  /* 16 bit LSB of Offset to procedure entry point */
	unsigned short	igd_segsel;   /* Segment selector */
	unsigned int	igd_rsvd : 5;  /* Reserved */
	unsigned int	igd_mbz : 3;   /* Must be 000 */
	unsigned int	igd_type : 5;  /* Tells whether Task gate, interrupt gate or trap gate */
	unsigned int	igd_dpl : 2;  /* Descriptor privelege level */
	unsigned int	igd_present : 1;   /* Segment present flag */
	unsigned short	igd_hoffset;    /* 16 bit MSB of offset to procedure entry point */
};

#define	IGDT_TASK	 5	/* task gate IDT descriptor		*/
#define	IGDT_INTR	14	/* interrupt gate IDT descriptor	*/
#define	IGDT_TRAPG	15	/* Trap Gate				*/

/*
idt.igd_type value xDxxx , D represents size of gate. D = 1 then 32 bits, D=0 then 16 bits 
IGDT_TASK 5 =   00101
IGDT_INTR 14 =  01110
IGDT_TRAPG 15 = 01111 
*/
/* Segment Descriptor	*/
/*A segment descriptor is a data structure in a GDT or LDT 
that provides the processor with the size and location of
a segment, as well as access control and status information */
/* See Intel Vol 3 Ch 3	*/
struct sd {
	unsigned short	sd_lolimit;     /* Segment limit 0:15 */
	unsigned short	sd_lobase;      /*Base address 16:32 */
	unsigned char	sd_midbase;     /* Base 23:16 */
	unsigned int	sd_perm : 3;    /* ? Maybe it's 3 LSB bits of Type- segment type */
	unsigned int	sd_iscode : 1;  /* ? Maybe it's MSB of TYPE-segment type*/
	unsigned int	sd_isapp : 1;   /* ? Maybe S - Descriptor type 0=System, 1= code or data */
	unsigned int	sd_dpl : 2;     /*Descriptor privelege level*/
	unsigned int	sd_present : 1; /* Segment present*/
	unsigned int	sd_hilimit : 4; /* Segment Limit 19:16 */
	unsigned int	sd_avl : 1;     /* Available for use by system software */
	unsigned int	sd_mbz : 1;		/* must be '0' */
	unsigned int	sd_32b : 1;     /*Default operation size 0=16 bit segment; 1 = 32 bit segment*/
	unsigned int	sd_gran : 1;    /*G- Granularity*/
	unsigned char	sd_hibase;      /* base 24:31 */
};

/* Task State Segment	*/
/* See Intel Vol 3 Ch 6 */
/* See figure 7-2. 32 bit task state segment */
/*
A task is a unit of work that a processor can dispatch, 
execute, and suspend. It can be used to execute a program,
a task or process, an operating-system service utility, 
an interrupt or exception handler, or a kernel or executive
utility.
In my opinion as Process is to an Operating system a Task 
is to the hardware.

A task is identified by the segment selector for its TSS. 
When a task is loaded into the processor for execution, the
segment selector, base address, limit, and segment 
descriptor attributes for the TSS are loaded into the task
register (see Section 2.4.4, “Task Register (TR)”).

If paging is implemented for the task, the base address of 
the page directory used by the task is loaded into control
register CR3.

CR2 — Contains the page-fault linear address (the linear address that caused a page fault).
CR3 — Contains the physical address of the base of the paging-structure hierarchy and two flags (PCD and
PWT).
Control register CR3 is also known as the page-directory base register (PDBR).
*/
struct tss {
  unsigned short	ts_ptl;
  unsigned short	ts_mbz12;
  unsigned int		ts_esp0;	/* use at pl 0 */
  unsigned short	ts_ss0;
  unsigned short	ts_mbz11;
  unsigned int		ts_esp1;	/* use at pl 1 */
  unsigned short	ts_ss1;
  unsigned short	ts_mbz10;
  unsigned int		ts_esp2;	/* use at pl 2*/
  unsigned short	ts_ss2;
  unsigned short	ts_mbz9;
  unsigned int		ts_pdbr;
  unsigned int		ts_eip;
  unsigned int		ts_efl;
  unsigned int		ts_eax;
  unsigned int		ts_ecx;
  unsigned int		ts_edx;
  unsigned int		ts_ebx;
  unsigned int		ts_esp;
  unsigned int		ts_ebp;
  unsigned int		ts_esi;
  unsigned int		ts_edi;
  unsigned short	ts_es;
  unsigned short	ts_mbz8;
  unsigned short	ts_cs;
  unsigned short	ts_mbz7;
  unsigned short	ts_ss;
  unsigned short	ts_mbz6;
  unsigned short	ts_ds;
  unsigned short	ts_mbz5;
  unsigned short	ts_fs;
  unsigned short	ts_mbz4;
  unsigned short	ts_gs;
  unsigned short	ts_mbz3;
  unsigned short	ts_ldtss;
  unsigned short	ts_mbz2;
  unsigned int		ts_t : 1;
  unsigned int		ts_mbz1 : 15;
  unsigned short	ts_ioba;
};

extern struct tss i386_tasks[];

#define	sd_type		sd_perm

/* System Descriptor Types */

#define	SDT_INTG	14	/* Interrupt Gate	*/

/* Segment Table Register */
struct segtr {
	unsigned int	len : 16;
	unsigned int	addr : 32;
};

/*
 * Delay units are in microseconds.
 */
#define	DELAY(n)					\
{							\
        extern int cpudelay;				\
        register int i;					\
	register long N = (((n)<<4) >> cpudelay);	\
 							\
	for (i=0;i<=4;i++)				\
	   {						\
	   N = (((n) << 4) >> cpudelay);		\
	   while (--N > 0) ;				\
	   }						\
}
