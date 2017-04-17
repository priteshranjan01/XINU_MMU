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

void proc1_test1(char *msg, int lck) {
	char *addr;
	int i;

	get_bs(TEST1_BS, 100);

	if (xmmap(PROC1_VPNO, TEST1_BS, 100) == SYSERR) {
		kprintf("xmmap call failed\n");
		sleep(1);
		return;
	}

	addr = (char*) PROC1_VADDR;
	for (i = 0; i < 26; i++) {
		*(addr + i * NBPG) = 'A' + i;
	}

//	sleep(1);

	for (i = 0; i < 26; i++) {
		kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
   
  kprintf("\nNow there should be no page faults");
  for (i = 0; i < 26; i++) {
		kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + 25 * NBPG));}
   
	}


	xmunmap(PROC1_VPNO);
 release_bs(TEST1_BS);
	return;
}



void proc1_test2(char *msg, int lck) {
	
int pid = currpid;
int *x;

	kprintf("ready to allocate heap space\n");
	x = vgetmem(1024);
	kprintf("heap allocated at %x\n", x);
	kprintf("ready to allocate heap space\n");
	struct mblock * mptr = proctab[pid].vmemlist->mnext;
	mptr->mnext = (struct mblock*)NULL;
	mptr->mlen = 100 * NBPG;
// I hope i didn't miss anything. Let's do a Litmus test
	kprintf("\npid = %d \t vmemlist->mnext 0x%08x  \t vmemlist->mlen %d",pid, (unsigned)(proctab[pid].vmemlist->mnext), proctab[pid].vmemlist->mlen);
	kprintf("\n mptr 0x%08x  size = %d ",(unsigned)mptr, mptr->mlen);

*x = 100;
	*(x + 1) = 200;
	kprintf("heap variable: %d %d\n", *x, *(x + 1));
vfreemem(x, 1024);

kprintf("If now you fail then kill is to blame");

}


int main() {
	int pid1;
	int pid2;

kprintf("\n1: shared memory\n");
	pid1 = create(proc1_test1, 2000, 20, "proc1_test1", 0, NULL);
	resume(pid1);
	sleep(10);
//test_sc_queue();
//kill(pid1);
 kprintf("\n2: vgetmem/vfreemem\n");
	pid1 = vcreate(proc1_test2, 2000, 100, 20, "proc1_test2", 0, NULL);
	kprintf("pid %d has private heap\n", pid1);
	resume(pid1);
	sleep(3);
kill(pid1);
 kprintf("\n2: vgetmem/vfreemem\n");
	pid1 = vcreate(proc1_test2, 2000, 100, 20, "proc1_test2", 0, NULL);
	kprintf("pid %d has private heap\n", pid1);
	resume(pid1);
	sleep(3);
kill(pid1);
 shutdown();
 }