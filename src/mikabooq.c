 /****************************************************************************************
  * Copyright 2017, Alberto Drusiani, Gabriele Calarota, Giorgio Valentini, Davide Nanni *
  																				 *
  * This file is part of Mikaboo.                                              			 *
  *                                                                             		 *
  * Mikaboo is free software: you can redistribute it and/or modify            			 *
  * it under the terms of the GNU General Public License as published by        		 *
  * the Free Software Foundation, either version 2 of the License, or          		 *
  * (at your option) any later version.                                         		 *
  *                                                                             		 *
  * Mikaboo is distributed in the hope that it will be useful,                 			 *
  * but WITHOUT ANY WARRANTY; without even the implied warranty of              		 *
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               		 *
  * GNU General Public License for more details.                                		 *
  *                                                                             		 *
  * You should have received a copy of the GNU General Public licenses          		 *
  * along with Mikaboo.  If not,write to the Free Software
  * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA . *
  ****************************************************************************************/



#include <mikabooq.h>
#include <listx.h>
#include <stdint.h>
#include <const.h>
#include <config.h>

struct pcb_t pcb_vect[MAXPROC];
struct list_head free_list_pcb;   
struct tcb_t tcb_vect[MAXTHREAD];
struct list_head free_list_tcb;
struct msg_t msg_vect[MAXMSG]; 
struct list_head free_list_msg;

/************************************** PROC MGMT ************************/

/* Initialize the data structure
 * The return value is the address of the root process */

struct pcb_t *proc_init(void){
	/* Creating the root process */
	struct pcb_t *pcb_root = pcb_vect;
	
	/* Initializing root processes fields*/
	pcb_root->p_parent=NULL;
	INIT_LIST_HEAD(&(pcb_root->p_threads));
	INIT_LIST_HEAD(&(pcb_root->p_children));		
	INIT_LIST_HEAD(&(pcb_root->p_siblings));	

	/* Initializing free processes list*/
	INIT_LIST_HEAD(&free_list_pcb);
	int i;
	for (i=1;i<(MAXPROC);i++){
		list_add(&(pcb_vect[i].p_siblings),&free_list_pcb);
		}
	return (pcb_root);
}

/* Alloc a new empty pcb (as a child of p_parent)
 * The return value is the address of the new process */

struct pcb_t *proc_alloc(struct pcb_t *p_parent){
	/* Check if there is a process available to alloc */
	if ( (p_parent == NULL) || (list_empty(&free_list_pcb)) ) {
			return NULL;
	}
	/* Get the first available process */
	struct list_head *iter=list_next(&free_list_pcb);
	struct pcb_t *new_pcb=container_of(iter,struct pcb_t,p_siblings); 

	list_del(iter);  

	INIT_LIST_HEAD(&(new_pcb->p_threads));
	INIT_LIST_HEAD(&(new_pcb->p_children));
	INIT_LIST_HEAD(&(new_pcb->p_siblings));
			
	new_pcb->p_parent=p_parent;
	
	/* Add the process as a child of p_parent */
	list_add(&(new_pcb->p_siblings),&(p_parent->p_children));
	return (new_pcb);
}

/* Delete a process (properly updating the process tree links)
 * this function must fail if the process has threads or children
 * The return value is 0 in case of success, -1 otherwise */

int proc_delete(struct pcb_t *oldproc){
	/* Check if the process can be deleted */
	if (oldproc == NULL){
		return -1;
	}
	if ( (!list_empty(&(oldproc->p_children))) || (!list_empty(&(oldproc->p_threads))) ){
		return -1;
	}
	
	struct pcb_t* old_proc_parent=oldproc->p_parent;
	
	/* Root cannot be deleted */
	if (old_proc_parent == NULL){
		return -1;
	}
	
	list_del(&(oldproc->p_siblings));
	
	/* Add the process to the free list */
	list_add(&(oldproc->p_siblings),&free_list_pcb);
	return 0;
}


/* Return the pointer to the first child (NULL if the process has no children or the process is NULL) */

struct pcb_t *proc_firstchild(struct pcb_t *proc){
	if (proc == NULL){
		return NULL;
	}
	if (list_empty(&(proc->p_children))){
		return NULL;
	}
	struct list_head *iter=list_prev(&(proc->p_children));
	struct pcb_t *first_child = container_of(iter,struct pcb_t,p_siblings);  
	return (first_child);
}

/* Return the pointer to the first thread (NULL if the process has no threads or the process is NULL) */

struct tcb_t *proc_firstthread(struct pcb_t *proc){
	/* Check if process is NULL */
	if (proc == NULL){
		return NULL;
	}
	/* Check if there is at least one thread */
	if (list_empty(&(proc->p_threads))){
		return NULL;
	}
	struct list_head *iter=list_prev(&(proc->p_threads));
	/* Get the thread */
	struct tcb_t *first_thread = container_of(iter,struct tcb_t,t_next);
	return first_thread;
}


/****************************************** THREAD ALLOCATION ****************/


/* Initialize the data structure */

void thread_init(){
	INIT_LIST_HEAD(&free_list_tcb);
	int i;
	for (i=0;i<(MAXTHREAD);i++){
		list_add(&(tcb_vect[i].t_next),&free_list_tcb);
	}
}


/* Alloc a new thread (as a thread of process)
 * return -1 if process == NULL or no more available threads
 * return 0 (success) otherwise */
	 
struct tcb_t *thread_alloc(struct pcb_t *process){
	if (process == NULL){
		return NULL;
	}
	/* Check if there are more threads available */
	if (list_empty(&free_list_tcb)) {
		return NULL;
	}
	
	struct list_head *iter=list_next(&free_list_tcb);
	struct tcb_t *new_tcb=container_of(iter,struct tcb_t,t_next); 
	list_del(iter);
	new_tcb->t_pcb=process;
	/* Initialize new thread */
	INIT_LIST_HEAD(&(new_tcb->t_sched));
	INIT_LIST_HEAD(&(new_tcb->t_msgq));
	/* Add the new thread */			 
	list_add(&(new_tcb->t_next),&(process->p_threads));
	return (new_tcb);
}

/* Deallocate a thread (unregistering it from the list of threads of its process)
 * return -1 if the message queue is not empty or the thread is NULL
 * return 0 otherwise */

int thread_free(struct tcb_t *oldthread){
	if (oldthread == NULL){
		return -1;
	}
	if (!list_empty(&(oldthread->t_msgq))){
		return -1;
	} 
	/* Delete the thread */
	list_del(&(oldthread->t_next));
	/* Add the thread to the free list */
	list_add(&(oldthread->t_next),&free_list_tcb);
	return 0;
}

/*************************** THREAD QUEUE ************************/


/* Add a thread to the scheduling queue */

void thread_enqueue(struct tcb_t *new, struct list_head *queue){
	if ((queue != NULL) && (new != NULL)){
		list_add(&(new->t_sched),queue);
	}
}

/* Return the head element of a scheduling queue
 * This function does not dequeues the element)
 * return NULL if the list is empty */

struct tcb_t *thread_qhead(struct list_head *queue){
	if (queue == NULL){
		return NULL;
	}
	if (list_empty(queue)){
		return NULL;
	}
	struct list_head *iter=list_prev(queue);
	struct tcb_t *tcb=container_of(iter,struct tcb_t,t_sched);
	return tcb;
}

/* Get the first element of a scheduling queue.
 * return NULL if the list is empty */

struct tcb_t *thread_dequeue(struct list_head *queue){
	if (queue == NULL){
		return NULL;
	}
	if (list_empty(queue)){
		return NULL;
	}
	struct list_head *iter=list_prev(queue);
	struct tcb_t *tcb=container_of(iter,struct tcb_t,t_sched);
	list_del(iter);
	return tcb;
}

/*************************** MSG QUEUE ************************/

/* Initialize the data structure */

void msgq_init(){
	INIT_LIST_HEAD(&free_list_msg);
	int i;
	for (i=0;i<(MAXMSG);i++){
		list_add(&(msg_vect[i].m_next),&free_list_msg);
	}
}


/* Add a message to a message queue 
 * return -1 if no more msgq elements are available 
 * return 0 otherwise */

int msgq_add(struct tcb_t *sender, struct tcb_t *destination, uintptr_t value){
	if ((sender==NULL)||(destination==NULL)){
		return -1;
	}
	if (list_empty(&free_list_msg)){
		return -1;
	}
	struct list_head *iter=list_next(&free_list_msg);
	struct msg_t *new_msg=container_of(iter,struct msg_t,m_next);
	list_del(iter);
	new_msg->m_sender=sender;
	new_msg->m_value=value;
	list_add_tail(&(new_msg->m_next),&(destination->t_msgq));
	return 0;
}

/* Retrieve a message from a message queue
 * if sender == NULL: get a message from any sender
 * if sender != NULL && *sender == NULL: get a message from any sender and store the address of the sending tcb in *sender
 * if sender != NULL && *sender != NULL: get a message sent by *sender
 * return -1 if there are no messages in the queue matching the request
 * return 0 and store the message payload in *value otherwise */

int msgq_get(struct tcb_t **sender, struct tcb_t *destination, uintptr_t *value){
	if ((destination == NULL) || (value == NULL)){
		return -1;
	}
	if (list_empty(&(destination->t_msgq))){
		return -1;
	}
	if (sender == NULL){
		struct list_head *found=list_next(&(destination->t_msgq));
		struct msg_t *new_msg=container_of(found,struct msg_t,m_next);
		list_del(found);
		*value = new_msg->m_value;
		list_add(&(new_msg->m_next),&free_list_msg);
		return 0;
	}
	if (*sender == NULL){
		struct list_head *iter=list_next(&(destination->t_msgq));
		struct msg_t *new_msg=container_of(iter,struct msg_t,m_next);
		list_del(iter);
		*value=new_msg->m_value;
		*sender=new_msg->m_sender;
		list_add(&(new_msg->m_next),&free_list_msg);
		return 0;
	}
	struct list_head *iter;
	list_for_each(iter,&(destination->t_msgq)){
		struct msg_t *new_msg=container_of(iter,struct msg_t,m_next);
		if (new_msg->m_sender == *sender){
			list_del(iter);
			*value=new_msg->m_value;
			list_add(&(new_msg->m_next),&free_list_msg);
			return 0;
		}
	}
	return -1;
}

