// #ifdef MM_PAGING
/*
 * PAGING based Memory Management
 * Virtual memory module mm/mm-vm.c
 */

#include "string.h"
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

/*get_vma_by_num - get vm area by numID
 *@mm: memory region
 *@vmaid: ID vm area to alloc memory region
 *
 */
struct vm_area_struct *get_vma_by_num(struct mm_struct *mm, int vmaid)
{
  struct vm_area_struct *pvma = mm->mmap;

  if (mm->mmap == NULL)
    return NULL;

  int vmait = pvma->vm_id;

  while (vmait < vmaid)
  {
    if (pvma == NULL)
      return NULL;

    pvma = pvma->vm_next;
    vmait = pvma->vm_id;
  }

  return pvma;
}

int __mm_swap_page(struct pcb_t *caller, int vicfpn , int swpfpn)
{
    __swap_cp_page(caller->mram, vicfpn, caller->active_mswp, swpfpn);
    return 0;
}

/*get_vm_area_node - get vm area for a number of pages
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@incpgnum: number of page
 *@vmastart: vma end
 *@vmaend: vma end
 *
 */
struct vm_rg_struct *get_vm_area_node_at_brk(struct pcb_t *caller, int vmaid, int size, int alignedsz)
{
  if (caller == NULL || caller->mm == NULL || size <= 0 || alignedsz < size) {
    return NULL;
  }
  /* TODO retrive current vma to obtain newrg, current comment out due to compiler redundant warning*/ //done
  //struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
    if (cur_vma == NULL || cur_vma->sbrk + alignedsz > cur_vma->vm_end) {
        return NULL;
    }

    struct vm_rg_struct *newrg = malloc(sizeof(struct vm_rg_struct));
    if (newrg == NULL) {
        return NULL;
    }


  /* TODO: update the newrg boundary //done
  // newrg->rg_start = ...
  // newrg->rg_end = ...
  */
  newrg->rg_start = cur_vma->sbrk;
  newrg->rg_end = newrg->rg_start + alignedsz - 1;
  cur_vma->sbrk += alignedsz;
  
  return newrg;
}

/*validate_overlap_vm_area
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@vmastart: vma end
 *@vmaend: vma end
 *
 */
int validate_overlap_vm_area(struct pcb_t *caller, int vmaid, int vmastart, int vmaend)
{
  if (caller == NULL || caller->mm == NULL) {
    return -1;  
  }

  if (vmastart >= vmaend) {
    return -1; 
  }
  
  struct vm_area_struct *vma = caller->mm->mmap;
  /* TODO validate the planned memory area is not overlapped */ //done
  while (vma != NULL) {
    // Skip the VMA we're checking against (if it exists)
    if (vma->vm_id != vmaid) {
        // Check for overlap conditions
        if ((vmastart >= vma->vm_start && vmastart < vma->vm_end) ||
            (vmaend > vma->vm_start && vmaend <= vma->vm_end) ||
            (vmastart <= vma->vm_start && vmaend >= vma->vm_end)) {
            return -1;  // Overlap detected
        }
    }
    vma = vma->vm_next;
  }
  return 0;
}

/*inc_vma_limit - increase vm area limits to reserve space for new variable
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@inc_sz: increment size
 *
 */
int inc_vma_limit(struct pcb_t *caller, int vmaid, int inc_sz)
{
  int inc_amt = PAGING_PAGE_ALIGNSZ(inc_sz);
  int incnumpage =  inc_amt / PAGING_PAGESZ;
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  if (cur_vma == NULL) {
    return -1; 
  }
  int old_end = cur_vma->vm_end;

  struct vm_rg_struct *area = get_vm_area_node_at_brk(caller, vmaid, inc_sz, inc_amt);
  if (area == NULL) {
      return -1;  
  }

  /*Validate overlap of obtained region */
  if (validate_overlap_vm_area(caller, vmaid, area->rg_start, area->rg_end) < 0) {
    free(area);
    return -1; /*Overlap and failed allocation */
  }

  struct vm_rg_struct *newrg = malloc(sizeof(struct vm_rg_struct));
  if (newrg == NULL) {
      free(area);
      return -1;  
  }
  /* TODO: Obtain the new vm area based on vmaid */ //done
  //cur_vma->vm_end... 
  // inc_limit_ret...
  cur_vma->vm_end += inc_amt;

  if (vm_map_ram(caller, area->rg_start, area->rg_end, 
                    old_end, incnumpage , newrg) < 0) {
    free(area);
    free(newrg);
    cur_vma->vm_end = old_end;  
    return -1; /* Map the memory to MEMRAM */
  }
  free(area);
  free(newrg);
  return 0;
}

// #endif
