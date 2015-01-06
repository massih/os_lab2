/* Tests cetegorical mutual exclusion with different numbers of threads.
 * Automatic checks only catch severe problems like crashes.
 */
#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "lib/random.h" //generate random numbers

#define BUS_CAPACITY 3
#define SENDER 0
#define RECEIVER 1
#define NORMAL 0
#define HIGH 1

/*
 *  initialize task with direction and priority
 *  call o
 * */
typedef struct {
    int direction;
    int priority;
} task_t;

// 00000000000000000000000000000000
struct semaphore mutex, semHigh, semReceive,semSend;
int bus_counter, dir, highPriorities, allSendTask, allReceiveTask;
// 00000000000000000000000000000000

void batchScheduler(unsigned int num_tasks_send, unsigned int num_task_receive,
        unsigned int num_priority_send, unsigned int num_priority_receive);

void senderTask(void *);
void receiverTask(void *);
void senderPriorityTask(void *);
void receiverPriorityTask(void *);


void oneTask(task_t task);/*Task requires to use the bus and executes methods below*/
void getSlot(task_t task); /* task tries to use slot on the bus */
void transferData(task_t task); /* task processes data on the bus either sending or receiving based on the direction*/
void leaveSlot(task_t task); /* task release the slot */



/* initializes semaphores */ 
void init_bus(void){ 
 
    random_init((unsigned int)123456789); 
    
    sema_init (&mutex, 1);
    sema_init (&semHigh, 0);
    sema_init (&semSend, 0);
    sema_init (&semReceive,0);
    bus_counter = 0;
    highPriorities = 0;
    dir = -1;
    allReceiveTask = 0;
    allSendTask = 0;
    // msg("NOT IMPLEMENTED START &&&*&&");
    /* FIXME implement */

}

/*
 *  Creates a memory bus sub-system  with num_tasks_send + num_priority_send
 *  sending data to the accelerator and num_task_receive + num_priority_receive tasks
 *  reading data/results from the accelerator.
 *
 *  Every task is represented by its own thread. 
 *  Task requires and gets slot on bus system (1)
 *  process data and the bus (2)
 *  Leave the bus (3).
 */

void batchScheduler(unsigned int num_task_send, unsigned int num_task_receive,
        unsigned int num_priority_send, unsigned int num_priority_receive)
{
    unsigned int i;
    char useless = 's'; 
    for(i=0; i < num_task_send; i++){
        thread_create("senderTask",PRI_MIN, &senderTask, &useless);
    }
    for(i=0; i < num_task_receive; i++){
        thread_create("receiverTask",PRI_MIN, &receiverTask, &useless);
    }
    for(i=0; i < num_priority_send; i++){
        thread_create("senderTaskHigh",PRI_MAX, &senderPriorityTask, &useless);
    }
    for(i=0; i < num_priority_receive; i++){
        thread_create("receiverTaskHigh",PRI_MAX, &receiverPriorityTask, &useless);
    }
    // msg("NOT IMPLEMENTED WHERE ???");
    /* FIXME implement */
}

/* Normal task,  sending data to the accelerator */
void senderTask(void *aux UNUSED){
        task_t task = {SENDER, NORMAL};
        oneTask(task);
}

/* High priority task, sending data to the accelerator */
void senderPriorityTask(void *aux UNUSED){
        task_t task = {SENDER, HIGH};
        oneTask(task);
}

/* Normal task, reading data from the accelerator */
void receiverTask(void *aux UNUSED){
        task_t task = {RECEIVER, NORMAL};
        oneTask(task);
}

/* High priority task, reading data from the accelerator */
void receiverPriorityTask(void *aux UNUSED){
        task_t task = {RECEIVER, HIGH};
        oneTask(task);
}

/* abstract task execution*/
void oneTask(task_t task) {
    getSlot(task);
    transferData(task);
    leaveSlot(task);
}


/* task tries to get slot on the bus subsystem */
void getSlot(task_t task) 
{
    sema_down(&mutex);
    if( dir == -1 || bus_counter == 0){
        dir = task.direction;
    }
    if(task.priority == HIGH){
        highPriorities ++;
    }else{
        if(task.direction == SENDER){
            allSendTask++;
        }else{
            allReceiveTask++;
        }
    }
    sema_up(&mutex);

    if(task.priority == NORMAL){
        if(bus_counter >= 3 || highPriorities > 0 || dir != task.direction){
            if(task.direction == SENDER){
                sema_down(&semSend);
            }else{
                sema_down(&semReceive);
            }
        }
    }else{
        if((bus_counter != 0 && dir != task.direction) || bus_counter >= 3){
            sema_down(&semHigh);
            sema_down(&mutex);
            dir = task.direction;
            sema_up(&mutex);
        }else{
            sema_down(&mutex);
            highPriorities--;
            sema_up(&mutex);
        }
    }
    

    sema_down(&mutex);
    bus_counter++;
    sema_up(&mutex);
    // msg("NOT IMPLEMENTED");
    /* FIXME implement */
}

/* task processes data on the bus send/receive */
void transferData(task_t task) 
{   
    // printf("task is : %d\n",task.direction);
    // printf("p is :  %d\n",task.direction);
    // int64_t sleeptime = 10;
    // timer_sleep(sleeptime);
    // msg("TASK START");
    // printf("STARTED\n" );
    int64_t sleeptime = (int64_t)random_ulong();
    timer_sleep(sleeptime%60);
    // printf("Finished\n" );
    // msg("TASK FINITO");
    // printf("p is done :  %d\n",task.direction);
    // printf("task is Done: %d \n",task.direction);
    // msg("NOT IMPLEMENTED");
    /* FIXME implement */
}

/* task releases the slot */
void leaveSlot(task_t task) 
{
    sema_down(&mutex);
    bus_counter --;
    if(task.priority == NORMAL){
        if(task.direction == SENDER){
            allSendTask --;
        }else{
            allReceiveTask --;
        }
    }
    if(bus_counter < 3){
        if(highPriorities > 0){
            if(bus_counter == 0){
                highPriorities --;
                sema_up(&semHigh);
            }
        }else{
            if(task.direction == SENDER){
                if(allSendTask > 0){
                    sema_up(&semSend);
                }else if(bus_counter == 0){
                    dir = RECEIVER;
                    sema_up(&semReceive);
                }
            }else{
                if(allReceiveTask > 0){
                    sema_up(&semReceive);
                }else if(bus_counter == 0){
                    dir  = SENDER;
                    sema_up(&semSend);
                }
            }
        }
    }
    sema_up(&mutex);
    // msg("NOT IMPLEMENTED");
    /* FIXME implement */
}