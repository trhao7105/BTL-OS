#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
        /* TODO: put a new process to queue [q] */ //done
        if (q == NULL || proc == NULL || q->size >= MAX_QUEUE_SIZE) {
                return; 
            }
        q->proc[q->size] = proc;
        q->size++;
}

struct pcb_t * dequeue(struct queue_t * q) {
        /* TODO: return a pcb whose prioprity is the highest //done
         * in the queue [q] and remember to remove it from q
         * */
	if (empty(q)) {
                return NULL;
        }
            
        int max_idx = 0;
        int max_prio = q->proc[0]->prio; 
            
        for (int i = 1; i < q->size; i++) {
                if (q->proc[i]->prio > max_prio) {
                    max_prio = q->proc[i]->prio;
                    max_idx = i;
                }
        }
            
            struct pcb_t *selected = q->proc[max_idx];
            
        for (int i = max_idx; i < q->size - 1; i++) {
                q->proc[i] = q->proc[i + 1];
        }
            
        q->size--;
        return selected;
}

