#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <paging.h>


#define PROC1_VADDR	0x40000000
#define PROC1_VPNO      0x40000
#define PROC2_VADDR     0x80000000
#define PROC2_VPNO      0x80000
#define TEST1_BS	1



void print_memory_list(int pid)
{
	struct mblock *p, *q, *memlist;
	memlist = proctab[pid].vmemlist;
	kprintf("\nCurrent memory allocation. pid= %d",pid);
	for(q=memlist, p = memlist->mnext; p != (struct mblock*)NULL; q=p, p = p->mnext)
	{
		kprintf("\nq= 0x%08x, size=0x%x, next=0x%08x", q, q->mlen, p);
	}
}

void proc1_test2(char *msg, int lck) {
	int *x, *y, *z;
	int status, pid, i;
	pid = currpid;
	struct mblock * memlist, *mptr;
	memlist = proctab[pid].vmemlist;
	mptr = memlist->mnext;
	print_memory_list(pid);
	kprintf("\nReady to allocate heap space");
	x = vgetmem(4096);
	print_memory_list(pid);
	y = vgetmem(4096);
	print_memory_list(pid);
	z = vgetmem(4096);
	print_memory_list(pid);
	kprintf("heap allocated at x= 0x%08x  y=0x%08x z=0x%08x\n ", x, y, z);
	
	kprintf("\nx= 0x%08x  y=0x%08x z=0x%08x\n ", ((unsigned)x)+1, ((unsigned)y)+1, ((unsigned)z)+1);

	for(i=0; i< 1024; i++)
	{
		*((unsigned*)x+i) = 'a';
	}
//*(((unsigned)x)+1) = 'Z';	
	for(i=0; i< 1024; i++)
	{
		*((unsigned*)y+i) = 'b';
	}
	for(i=0; i< 1024; i++)
	{
		*((unsigned*)z+i) = 'c';
	}

	for(i=0; i< 1024; i++)
	{
		kprintf(" %c ",*((unsigned*)x+i));
	}
	
	for(i=0; i< 1024; i++)
	{
		kprintf(" %c ",*((unsigned*)y+i));
	}
	for(i=0; i< 1024; i++)
	{
		kprintf(" %c ",*((unsigned*)z+i));
	}
	
	
	status = vfreemem(y, 1024);
	print_memory_list(pid);
	if (status == SYSERR)
		kprintf("\n1 vfreemem failed");

	status = vfreemem(x, 1024);
	print_memory_list(pid);
	if (status == SYSERR)
		kprintf("\nvfreemem failed");

	status = vfreemem(z, 1024);
	print_memory_list(pid);
	if (status == SYSERR)
		kprintf("\nvfreemem failed");

}


int main() {
	int pid1;
	int pid2;

	kprintf("\n2: vgetmem/vfreemem\n");
	pid1 = vcreate(proc1_test2, 2000, 100, 20, "proc1_test2", 0, NULL);
	kprintf("pid %d has private heap\n", pid1);
	resume(pid1);
	sleep(3);
 shutdown();
}
