/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
 
 void proc1_test1(char *msg, int lck) {
 kprintf("\n A walk around the lake");
 char *addr;
	int i;
	get_bs(TEST1_BS, 100);
	if (xmmap(PROC1_VPNO, TEST1_BS, 100) == SYSERR) {
		kprintf("xmmap call failed\n");
		sleep(3);
		return;
	}
 dummy_pfint(0xa0000000);
 }
 
int main()
{
	int pid1;
	kprintf("\n\nHello World, Xinu@QEMU lives\n\n");

        /* The hook to shutdown QEMU for process-like execution of XINU.
         * This API call terminates the QEMU process.
         */
	kprintf("\n1: shared memory\n");
	pid1 = create(proc1_test1, 2000, 25, "proc1_test1", 0, NULL);
	resume(pid1);
	sleep(4);
        shutdown();
}
