/* policy.c = srpolicy*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * srpolicy - set page replace policy 
 *-------------------------------------------------------------------------
 */
int sc_head  = -1;
int sc_tail = -1;

SYSCALL srpolicy(int policy)
{
	  pr_debug = TRUE;
	  switch(policy)
	  {
		case AGING:
		default:  /* Fall Through */
		case SC:
			sc_head = -1;
			sc_tail = -1;
	  }
	  page_replace_policy = policy;
	  
	  return OK;
 }

/*-------------------------------------------------------------------------
 * grpolicy - get page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL grpolicy()
{
  return page_replace_policy;
}
