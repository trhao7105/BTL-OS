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
    struct pcb_t *p;
    struct list_head *pos, *tmp;

    list_for_each_safe(pos, tmp, &caller->running_list) {
        p = list_entry(pos, struct pcb_t, list);
        if (strcmp(p->name, proc_name) == 0) {
            printf("Terminating process: %s (PID: %d)\n", p->name, p->pid);
            terminate_process(p);
        }
    }

    if (caller->mlq_ready_queue != NULL) {
        struct queue_t *queue = caller->mlq_ready_queue; 
        list_for_each_safe(pos, tmp, &queue->list) {
            p = list_entry(pos, struct pcb_t, list);
            if (strcmp(p->name, proc_name) == 0) {
                printf("Terminating process: %s (PID: %d)\n", p->name, p->pid);
                terminate_process(p);
            }
        }
    }

    return 0;
     
}
