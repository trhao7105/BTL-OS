/*
 * Copyright (C) 2025 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Sierra release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

#include "common.h"
#include "syscall.h"
#include "stdio.h"
#include "libmem.h"
#include "queue.h"

int __sys_killall(struct pcb_t *caller, struct sc_regs* regs)
{
    char proc_name[100];
    uint32_t data;

    //hardcode for demo only
    uint32_t memrg = regs->a1;
    
    /* TODO: Get name of the target proc */ //done
    //proc_name = libread..
    int i = 0;
    data = 0;
    while(data != -1){
        libread(caller, memrg, i, &data);
        proc_name[i]= data;
        if(data == -1) proc_name[i]='\0';
        i++;
    }
    proc_name[i] = '\0';
    printf("The procname retrieved from memregionid %d is \"%s\"\n", memrg, proc_name);

    /* TODO: Traverse proclist to terminate the proc
     *       stcmp to check the process match proc_name
     */
    //caller->running_list
    //caller->mlq_ready_queu

    /* TODO Maching and terminating 
     *       all processes with given
     *        name in var proc_name
     */
    if (caller->running_list != NULL) {
        for (int i = 0; i < caller->running_list->size; ++i) {
            struct pcb_t *p = caller->running_list->proc[i];
            if (p != NULL && strcmp(p->path, proc_name) == 0) {
                printf("Terminating process: %s (PID: %d)\n", p->path, p->pid);
                terminate_process(p);
            }
        }
    }

    // Duyệt các tiến trình trong MLQ nếu bật
#ifdef MLQ_SCHED
    if (caller->mlq_ready_queue != NULL) {
        for (int i = 0; i < caller->mlq_ready_queue->size; ++i) {
            struct pcb_t *p = caller->mlq_ready_queue->proc[i];
            if (p != NULL && strcmp(p->path, proc_name) == 0) {
                printf("Terminating process: %s (PID: %d)\n", p->path, p->pid);
                terminate_process(p);
            }
        }
    }
#endif

    return 0;
     
}
