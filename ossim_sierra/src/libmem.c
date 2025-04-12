/*
 * Copyright (C) 2025 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Sierra release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

// #ifdef MM_PAGING
/*
 * System Library
 * Memory Module Library libmem.c 
 */

#include "string.h"
#include "mm.h"
#include "syscall.h"
#include "libmem.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

static pthread_mutex_t mmvm_lock = PTHREAD_MUTEX_INITIALIZER;

/*enlist_vm_freerg_list - add new rg to freerg_list
 *@mm: memory region
 *@rg_elmt: new region
 *
 */
int enlist_vm_freerg_list(struct mm_struct *mm, struct vm_rg_struct *rg_elmt)
{
  struct vm_rg_struct *rg_node = mm->mmap->vm_freerg_list;

  if (rg_elmt->rg_start >= rg_elmt->rg_end)
    return -1;

  if (rg_node != NULL)
    rg_elmt->rg_next = rg_node;

  /* Enlist the new region */
  mm->mmap->vm_freerg_list = rg_elmt;

  return 0;
}

/*get_symrg_byid - get mem region by region ID
 *@mm: memory region
 *@rgid: region ID act as symbol index of variable
 *
 */
struct vm_rg_struct *get_symrg_byid(struct mm_struct *mm, int rgid)
{
  if (rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
    return NULL;

  return &mm->symrgtbl[rgid];
}

/*__alloc - allocate a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *@alloc_addr: address of allocated memory region
 *
 */
int __alloc(struct pcb_t *caller, int vmaid, int rgid, int size, int *alloc_addr)
{
  /*Allocate at the toproof */
  struct vm_rg_struct rgnode;

  /* TODO: commit the vmaid */ //done
  rgnode.vmaid = vmaid;

  if (get_free_vmrg_area(caller, vmaid, size, &rgnode) == 0)
  {
    caller->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
    caller->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;
 
    *alloc_addr = rgnode.rg_start;

    pthread_mutex_unlock(&mmvm_lock);
    return 0;
  }

  /* TODO get_free_vmrg_area FAILED handle the region management (Fig.6)*/

  /* TODO retrive current vma if needed, current comment out due to compiler redundant warning*/
  /*Attempt to increate limit to get space */
  //struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  if (cur_vma == NULL) { pthread_mutex_unlock(&mmvm_lock); return -1; }

  //int inc_sz = PAGING_PAGE_ALIGNSZ(size);
  //int inc_limit_ret;

  /* TODO retrive old_sbrk if needed, current comment out due to compiler redundant warning*/
  //int old_sbrk = cur_vma->sbrk;
  int inc_sz = PAGING_PAGE_ALIGNSZ(size);
  int inc_limit_ret;

  /* TODO retrive old_sbrk if needed, current comment out due to compiler redundant warning*/ //done
  int old_sbrk = cur_vma->sbrk;

  /* TODO INCREASE THE LIMIT as inovking systemcall 
   * sys_memap with SYSMEM_INC_OP 
   */
  //struct sc_regs regs;
  //regs.a1 = ...
  //regs.a2 = ...
  //regs.a3 = ...
  struct sc_regs regs;
  regs.a1 = (int)caller;
  regs.a2 = (int)vmaid;
  regs.a3 = (int)inc_sz;
  
  /* SYSCALL 17 sys_memmap */
  sys_memmap(SYSMEM_INC_OP, &regs);
  /* TODO: commit the limit increment */ //done
  cur_vma->sbrk += inc_sz;
  /* TODO: commit the allocation address  */ //done
  if (get_free_vmrg_area(caller, vmaid, size, &rgnode) == 0)
    {
        caller->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
        caller->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;
        *alloc_addr = rgnode.rg_start;
        
        pthread_mutex_unlock(&mmvm_lock);
        return 0;
    }

    /* If allocation still fails, restore old sbrk and return error */
    cur_vma->sbrk = old_sbrk;
    return -1;

}

/*__free - remove a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __free(struct pcb_t *caller, int vmaid, int rgid)
{
  //struct vm_rg_struct rgnode;

  // Dummy initialization for avoding compiler dummay warning
  // in incompleted TODO code rgnode will overwrite through implementing
  // the manipulation of rgid later

  if(rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
    return -1;

  /* TODO: Manage the collect freed region to freerg_list */
  struct vm_rg_struct *rgnode = get_symrg_byid(caller->mm, rgid);
  if (rgnode == NULL) return -1;
  struct vm_rg_struct *freed_rg = malloc(sizeof(struct vm_rg_struct));
  freed_rg->rg_start = rgnode->rg_start;
  freed_rg->rg_end = rgnode->rg_end;
  freed_rg->rg_next = NULL;

  /*enlist the obsoleted memory region */
  enlist_vm_freerg_list(caller, rgnode);
  /* Clear the region information */
  rgnode->rg_start = 0;
  rgnode->rg_end = 0;
  return 0;
}

/*liballoc - PAGING-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int liballoc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  /* TODO Implement allocation on vm area 0 */
  int addr;

  /* By default using vmaid = 0 */
  return __alloc(proc, 0, reg_index, size, &addr);
}

/*libfree - PAGING-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */

int libfree(struct pcb_t *proc, uint32_t reg_index)
{
  /* TODO Implement free region */

  /* By default using vmaid = 0 */
  return __free(proc, 0, reg_index);
}

/*pg_getpage - get the page in ram
 *@mm: memory region
 *@pagenum: PGN
 *@framenum: return FPN
 *@caller: caller
 *
 */
int pg_getpage(struct mm_struct *mm, int pgn, int *fpn, struct pcb_t *caller)
{
  uint32_t pte = mm->pgd[pgn];

  if (!PAGING_PAGE_PRESENT(pte))
  { /* Page is not online, make it actively living */
    int vicpgn, swpfpn; 
    //int vicfpn;
    //uint32_t vicpte;

    //int tgtfpn = PAGING_PTE_SWP(pte);//the target frame storing our variable
    int tgtfpn = PAGING_PTE_SWP(pte);

    /* TODO: Play with your paging theory here */
    /* Find victim page */
    find_victim_page(caller->mm, &vicpgn);
    uint32_t vicpte = mm->pgd[vicpgn];
    int vicfpn = PAGING_PTE_FPN(vicpte);
    /* Get free frame in MEMSWP */
    // MEMPHY_get_freefp(caller->active_mswp, &swpfpn);
    if (MEMPHY_get_freefp(caller->active_mswp, &swpfpn) != 0) return -1;

    /* TODO: Implement swap frame from MEMRAM to MEMSWP and vice versa*/

    /* TODO copy victim frame to swap 
     * SWP(vicfpn <--> swpfpn)
     * SYSCALL 17 sys_memmap 
     * with operation SYSMEM_SWP_OP
     */
    //struct sc_regs regs;
    //regs.a1 =...
    //regs.a2 =...
    //regs.a3 =..
    struct sc_regs regs;
    regs.a1 = (int)caller->mram;        
    regs.a2 = vicfpn;                    
    regs.a3 = (int)caller->active_mswp;  
    regs.a4 = swpfpn;                    
    regs.a5 = SYSMEM_SWP_OP;          
    sys_memmap(&regs);                  

    /* SYSCALL 17 sys_memmap */
    syscall(caller, 17, &regs);
    /* TODO copy target frame form swap to mem 
     * SWP(tgtfpn <--> vicfpn)
     * SYSCALL 17 sys_memmap
     * with operation SYSMEM_SWP_OP
     */
    /* TODO copy target frame form swap to mem */
    regs.a1 = (int)caller->active_mswp;  
    regs.a2 = tgtfpn;                   
    regs.a3 = (int)caller->mram;         
    regs.a4 = vicfpn;                   
    regs.a5 = SYSMEM_SWP_OP;             
    sys_memmap(&regs);                  

    /* SYSCALL 17 sys_memmap */
    syscall(caller, 17, &regs);

    /* Update page table */
    //pte_set_swap() 
    //mm->pgd;
    pte_set_swap(&mm->pgd[vicpgn], 0, swpfpn);

    /* Update its online status of the target page */
    //pte_set_fpn() &
    //mm->pgd[pgn];
    //pte_set_fpn();
    pte_set_fpn(&mm->pgd[pgn], vicfpn);

    enlist_pgn_node(&caller->mm->fifo_pgn,pgn);
  }

  *fpn = PAGING_FPN(mm->pgd[pgn]);

  return 0;
}

/*pg_getval - read value at given offset
 *@mm: memory region
 *@addr: virtual address to acess
 *@value: value
 *
 */
int pg_getval(struct mm_struct *mm, int addr, BYTE *data, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  //int off = PAGING_OFFST(addr);
  int off = PAGING_OFFST(addr);
  int fpn;

  /* Get the page to MEMRAM, swap from MEMSWAP if needed */
  if (pg_getpage(mm, pgn, &fpn, caller) != 0)
    return -1; /* invalid page access */

  /* TODO 
   *  MEMPHY_read(caller->mram, phyaddr, data);
   *  MEMPHY READ 
   *  SYSCALL 17 sys_memmap with SYSMEM_IO_READ
   */
  // int phyaddr
  //struct sc_regs regs;
  //regs.a1 = ...
  //regs.a2 = ...
  //regs.a3 = ...
  /* SYSCALL 17 sys_memmap */
  int phyaddr = (fpn << PAGING_ADDR_OFFST_HIBIT) + off;
  struct sc_regs regs;
  regs.a1 = (int)caller->mram;    
  regs.a2 = phyaddr;              
  regs.a3 = (int)data;           
  regs.a4 = 1;                  
  regs.a5 = SYSMEM_IO_READ;       
  
  sys_memmap(&regs);             

  // Update data
  // data = (BYTE)
  *data = (BYTE)regs.a3;

  return 0;
}

/*pg_setval - write value to given offset
 *@mm: memory region
 *@addr: virtual address to acess
 *@value: value
 *
 */
int pg_setval(struct mm_struct *mm, int addr, BYTE value, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  int off = PAGING_OFFST(addr);
  int fpn;

  /* Get the page to MEMRAM, swap from MEMSWAP if needed */
  if (pg_getpage(mm, pgn, &fpn, caller) != 0)
    return -1; /* invalid page access */

  /* TODO
   *  MEMPHY_write(caller->mram, phyaddr, value);
   *  MEMPHY WRITE
   *  SYSCALL 17 sys_memmap with SYSMEM_IO_WRITE
   */
  // int phyaddr
  //struct sc_regs regs;
  //regs.a1 = ...
  //regs.a2 = ...
  //regs.a3 = ...
  /* SYSCALL 17 sys_memmap */
  int phyaddr = (fpn << PAGING_ADDR_OFFST_HIBIT) + off;
  struct sc_regs regs;
  regs.a1 = (int)caller->mram;   
  regs.a2 = phyaddr;              
  regs.a3 = (int)&value;        
  regs.a4 = 1;                   
  regs.a5 = SYSMEM_IO_WRITE;     

  /* SYSCALL 17 sys_memmap */
  sys_memmap(&regs);
  // Update data
  // data = (BYTE) 
   
  return 0;
}

/*__read - read value in region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __read(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE *data)
{
  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  if (currg == NULL || cur_vma == NULL) /* Invalid memory identify */
    return -1;

  pg_getval(caller->mm, currg->rg_start + offset, data, caller);

  return 0;
}

/*libread - PAGING-based read a region memory */
int libread(
    struct pcb_t *proc, // Process executing the instruction
    uint32_t source,    // Index of source register
    uint32_t offset,    // Source address = [source] + [offset]
    uint32_t* destination)
{
  BYTE data;
  int val = __read(proc, 0, source, offset, &data);

  /* TODO update result of reading action*/
  //destination 
  *destination = data;  // Update destination register

#ifdef IODUMP
  printf("read region=%d offset=%d value=%d\n", source, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  return val;
}

/*__write - write a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __write(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE value)
{
  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  if (currg == NULL || cur_vma == NULL) /* Invalid memory identify */
    return -1;

  pg_setval(caller->mm, currg->rg_start + offset, value, caller);

  return 0;
}

/*libwrite - PAGING-based write a region memory */
int libwrite(
    struct pcb_t *proc,   // Process executing the instruction
    BYTE data,            // Data to be wrttien into memory
    uint32_t destination, // Index of destination register
    uint32_t offset)
{
#ifdef IODUMP
  printf("write region=%d offset=%d value=%d\n", destination, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  return __write(proc, 0, destination, offset, data);
}

/*free_pcb_memphy - collect all memphy of pcb
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@incpgnum: number of page
 */
int free_pcb_memph(struct pcb_t *caller)
{
  int pagenum, fpn;
  uint32_t pte;


  for(pagenum = 0; pagenum < PAGING_MAX_PGN; pagenum++)
  {
    pte= caller->mm->pgd[pagenum];

    if (!PAGING_PAGE_PRESENT(pte))
    {
      fpn = PAGING_PTE_FPN(pte);
      MEMPHY_put_freefp(caller->mram, fpn);
    } else {
      fpn = PAGING_PTE_SWP(pte);
      MEMPHY_put_freefp(caller->active_mswp, fpn);    
    }
  }

  return 0;
}


/*find_victim_page - find victim page
 *@caller: caller
 *@pgn: return page number
 *
 */
int find_victim_page(struct mm_struct *mm, int *retpgn)
{
  if (mm == NULL || retpgn == NULL || mm->fifo_pgn == NULL) {
    return -1;  // Invalid parameters or empty page list
  }
  struct pgn_t *pg = mm->fifo_pgn;

  /* TODO: Implement the theorical mechanism to find the victim page */ //done
  *retpgn = pg->pgn;  
    // Remove the victim from FIFO queue
    mm->fifo_pgn = pg->pg_next;
    if (mm->fifo_pgn != NULL) {
        mm->fifo_pgn->pg_prev = NULL;
    }

  free(pg);

  return 0;
}

/*get_free_vmrg_area - get a free vm region
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@size: allocated size
 *
 */
int get_free_vmrg_area(struct pcb_t *caller, int vmaid, int size, struct vm_rg_struct *newrg)
{
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  if (cur_vma == NULL) return -1;   //Tự điền
  struct vm_rg_struct *rgit = cur_vma->vm_freerg_list;
  if (rgit == NULL)
    return -1;

  /* Probe unintialized newrg */
  newrg->rg_start = newrg->rg_end = -1;

  /* TODO Traverse on list of free vm region to find a fit space */ //done
  while (rgit != NULL) {
    int region_size = rgit->rg_end - rgit->rg_start + 1;
    
    if (region_size >= size) {
        /* Found suitable region */
        newrg->rg_start = rgit->rg_start;
        newrg->rg_end = rgit->rg_start + size - 1;
        
        /* Update the free region list */
        if (region_size == size) {
            /* Exact fit - remove this region from free list */
            if (rgit->rg_prev) {
                rgit->rg_prev->rg_next = rgit->rg_next;
            } else {
                cur_vma->vm_freerg_list = rgit->rg_next;
            }
            
            if (rgit->rg_next) {
                rgit->rg_next->rg_prev = rgit->rg_prev;
            }
            free(rgit);
        } else {
            /* Partial fit - adjust the free region */
            rgit->rg_start += size;
        }
        
        return 0;  
      }
      rgit = rgit->rg_next;
    }
  return -1;
  // return 0;
}

//#endif
