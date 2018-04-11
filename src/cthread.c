#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <string.h>
#include "cdata.h"
#include "support.h"
#include "cthread.h"

FILA2 runQueue; // queue of threads that are ready to run
FILA2 blockedQueue; // queue of blocked threads
FILA2 sRunQueue; // queue of suspended threads that are ready to run
FILA2 sBlockedQueue; // queue of suspended blocked threads

FILA2 joinedThreads; // list of pairs (blockingThread, blockedThread)

TCB_t mainThread;
TCB_t *runningThread;

ucontext_t scheduler; // context to schedule and dispatch
char schedulerStack[SIGSTKSZ];

ucontext_t finalize; // context to finalize thread execution 
char finalizeStack[SIGSTKSZ];

int TID = 0;
int libraryInitialized = 0;

int getNextTid(){
	return ++TID;
}

// Returns 1 if the tid is on queue, 0 otherwise
int isTidOnQueue(PFILA2 queue, int tid){
    TCB_t *thread;

     // Put iterator at the beggining and returns if queue is empty
    if(FirstFila2(queue) != 0)
        return 0;

    do{
        thread = (TCB_t*)GetAtIteratorFila2(queue);
        if(tid == thread->tid)
            return 1;
    }while(NextFila2(queue) == 0);

    // Tid not found
    return 0;

}

int deleteTidOnQueue(PFILA2 queue, int tid){

    TCB_t *thread;

     // Put iterator at the beggining and returns if queue is empty
    if(FirstFila2(queue) != 0)
        return -1;

    do{
        thread = (TCB_t*)GetAtIteratorFila2(queue);
        if(tid == thread->tid){
            DeleteAtIteratorFila2(queue);
            return 0;
        }
    }while(NextFila2(queue) == 0);

    // Tid not found
    return -1;

}

// Removes thread from queue and returs thread pointer in returnThread
// ret: 0 == no error
//      else ERROR
int pullTidOnQueue(PFILA2 queue, int tid, TCB_t** returnThread){

    TCB_t *thread;

    // Put iterator at the beggining and returns if queue is empty
    if(FirstFila2(queue) != 0)
        return -1;

    do {
        thread = (TCB_t*)GetAtIteratorFila2(queue);
        if(tid == thread->tid){
            DeleteAtIteratorFila2(queue);
            (*returnThread) = thread;
            return 0;
        }
    }while(NextFila2(queue) == 0);

    // Tid not found
    return -1;
}

// It returns 1 if the given tid is already blocking another thread, 0 otherwise
int isJoined(int tid){
    pair *joinThreads;

     // Put iterator at the beggining and returns if list is empty
    if(FirstFila2(&joinedThreads) != 0)
        return 0;

    do{
        joinThreads = (pair*)GetAtIteratorFila2(&joinedThreads);
        if(tid == joinThreads->tid_blocking)
            return 1;
    }while(NextFila2(&joinedThreads) == 0);

    return 0;
}

// unblock thread that was waiting for the running thread
int unblockThread(){
    pair *joinThreads;
    TCB_t *blockedThread;

     // Put iterator at the beggining and returns if list is empty
    if(FirstFila2(&joinedThreads) != 0)
        return 0;

    do{
        joinThreads = (pair*)GetAtIteratorFila2(&joinedThreads);
        if(runningThread->tid == joinThreads->tid_blocking){
            // if thread was blocked and not suspended
            // remove thread from blockedQueue and put in runQueue
            if(pullTidOnQueue(&blockedQueue, joinThreads->tid_blocked, &blockedThread) == 0){
                if(AppendFila2(&runQueue, (void *)blockedThread) != 0){
                    printf("Error: insertion of the thread in the run queue failed");
                }
                return 0;
            }
            // if thread was blocked and suspended
            // remove thread from sBlockedQueue and put in sRunQueue
            else if(pullTidOnQueue(&sBlockedQueue, joinThreads->tid_blocked, &blockedThread) == 0){
                if(AppendFila2(&sRunQueue, (void *)blockedThread) != 0){
                    printf("Error: insertion of the thread in the suspended run queue failed");
                }
                return 0;
            }
        }
    }while(NextFila2(&joinedThreads) == 0);

    return -1;    
}

int schedule(){
    if(FirstFila2(&runQueue) != 0)
        return 0;

    runningThread = (TCB_t*)GetAtIteratorFila2(&runQueue);
    DeleteAtIteratorFila2(&runQueue);

    runningThread->state = PROCST_EXEC;
    setcontext(&runningThread->context);

    return 0;
}

// call function to free blocked threads
// free memory alocated to thread that has finished
// call the scheduler
int threadFinalized(){

    unblockThread();

    free(runningThread->context.uc_stack.ss_sp); //free thread stack
    free(runningThread); // free thread

    runningThread = NULL;
    
    schedule();

    return 0;
}

void initializeCThread(){
    if(libraryInitialized)
        return;

    if(CreateFila2(&runQueue) != 0)
        printf("Error: Run Queue initialization failed\n");
    if(CreateFila2(&sRunQueue) != 0)
        printf("Error: Suspend-Run Queue initialization failed\n");
    if(CreateFila2(&blockedQueue) != 0)
        printf("Error: Blocked Queue initialization failed\n");
    if(CreateFila2(&sBlockedQueue) != 0)
        printf("Error: Suspend-Blocked Queue initialization failed\n");

    mainThread.tid = 0;
    mainThread.prio = 0;
    mainThread.state = PROCST_EXEC;
    getcontext(&(mainThread.context));

    runningThread = &mainThread;

    getcontext(&scheduler);
    scheduler.uc_link = 0;
    scheduler.uc_stack.ss_sp = schedulerStack;
    scheduler.uc_stack.ss_size = SIGSTKSZ;
    makecontext(&scheduler, (void(*)(void))schedule, 0);

    getcontext(&finalize);
    finalize.uc_link = 0;
    finalize.uc_stack.ss_sp = finalizeStack;
    finalize.uc_stack.ss_size = SIGSTKSZ;
    makecontext(&finalize, (void(*)(void))threadFinalized, 0);

    libraryInitialized = 1;
}

int cidentify(char *name, int size){

    char *group = "Lucas Nunes Alegre 00274693\nAline Weber 00274720\nLucas Sonntag Hagen\n\0";
    if(size < strlen(group)){
        printf("Size given is not sufficient to copy the whole string!\n");
        return -1;
    }

    strncpy(name, group, size-1);
    name[size] = '\0';

    return 0;
}

int ccreate (void* (*start)(void*), void *arg, int prio){
    initializeCThread();

    TCB_t* createdThread = (TCB_t*)malloc(sizeof(TCB_t));
    createdThread->tid = getNextTid();   
    createdThread->prio = 0;
    createdThread->state = PROCST_APTO;

    getcontext(&(createdThread->context));
    createdThread->context.uc_link = &finalize;
    createdThread->context.uc_stack.ss_sp = (char*)malloc(SIGSTKSZ);
    createdThread->context.uc_stack.ss_size = SIGSTKSZ;
    makecontext(&(createdThread->context), (void(*)(void))start, 1, arg);

    if(AppendFila2(&runQueue, (void*)createdThread) != 0){
        printf("Error: insertion of the new thread in the Run Queue failed\n");
        return -1;
    }

    return createdThread->tid;
}

int cyield(void){
    TCB_t* threadYield;

    initializeCThread();

    // in case that there are no threads to run next
    if(FirstFila2(&runQueue) != 0)
        return 0;

    threadYield = runningThread;
    threadYield->state = PROCST_APTO;

    if(AppendFila2(&runQueue, (void*)threadYield) != 0){
        printf("Error: insertion of the thread back in the run queue failed\n");
        return -1;
    }

    runningThread = NULL;

    if(swapcontext(&threadYield->context, &scheduler) == -1)
        return -1;

    return 0;
}

int cjoin(int tid){
    pair *joinThreads = (pair*)malloc(sizeof(pair));
    TCB_t *blockedThread;

    initializeCThread();

    if(tid == 0){
        printf("Error: can not wait for main\n");
        return -1;
    }
    if(isJoined(tid)){
        printf("Error: can not wait for this tid because someone else is already waiting\n");
        return -1;
    }

    if(!isTidOnQueue(&blockedQueue, tid) && !isTidOnQueue(&sBlockedQueue, tid) 
        && !isTidOnQueue(&runQueue, tid) && !isTidOnQueue(&sRunQueue, tid)){
        printf("Error: can not wait for thread that has finished or does not exists\n");
        return -1;
    }

    joinThreads->tid_blocking = tid;
    joinThreads->tid_blocked = runningThread->tid;
    
    blockedThread = runningThread;
    blockedThread->state = PROCST_BLOQ;

    if(AppendFila2(&blockedQueue, (void *)blockedThread) != 0){
        printf("Error: insertion of the thread in the blocked queue failed\n");
        return -1;
    }

    if(AppendFila2(&joinedThreads, joinThreads) != 0){
        printf("Error: insertion of the pair in the joined threads list failed\n");
        return -1;
    }
    
    runningThread = NULL;
    
    if(swapcontext(&blockedThread->context, &scheduler) == -1){
        printf("Error: could not swap context\n");
        return -1;
    }

    return 0;
}

int csuspend(int tid) {
    initializeCThread();

    // Thread cant self suspend
    if (runningThread->tid == tid) {
        return -1;
    }

    // Check if thread is in run queue
    if (isTidOnQueue(&runQueue, tid)) {
        TCB_t* thread;

        // Tries to remove thread from queue (0 == no error)
        if(pullTidOnQueue(&runQueue, tid, &thread) == 0) {
            // Tries to append thread to queue
            if(AppendFila2(&sRunQueue, (void*)thread) == 0) {
                return 0;
            }
        }
        return -2;
    }

    // Check if thread is in blocked queue
    if (isTidOnQueue(&blockedQueue, tid)) {
        TCB_t* thread;

        // Tries to remove thread from queue (0 == no error)
        if(pullTidOnQueue(&blockedQueue, tid, &thread) == 0) {
            // Tries to append thread to queue
            if(AppendFila2(&sBlockedQueue, (void*)thread) == 0) {
                return 0;
            }
        }
        return -2;
    }

    return -3;

}

int cresume(int tid) {
    initializeCThread();

    // Thread cant self resume
    if (runningThread->tid == tid) {
        return -1;
    }

    // Check if thread is in suspended run queue
    if (isTidOnQueue(&sRunQueue, tid)) {
        TCB_t* thread;

        // Tries to remove thread from suspended queue (0 == no error)
        if(pullTidOnQueue(&sRunQueue, tid, &thread) == 0) {
            // Tries to append thread to queue
            if(AppendFila2(&runQueue, (void*)thread) == 0) {
                return 0;
            }
        }
        return -2;
    }

    // Check if thread is in suspended blocked queue
    if (isTidOnQueue(&sBlockedQueue, tid)) {
        TCB_t* thread;

        // Tries to remove thread from suspended queue (0 == no error)
        if(pullTidOnQueue(&sBlockedQueue, tid, &thread) == 0) {
            // Tries to append thread to queue
            if(AppendFila2(&blockedQueue, (void*)thread) == 0) {
                return 0;
            }
        }
        return -2;
    }

    return -3;

}

int csem_init(csem_t *sem, int count){
    initializeCThread();

    sem->count = count;
    if(CreateFila2(sem->fila) == 0){
        return 0;
    }
    else{
        printf("Error: Semaphore queue creation failed\n");
        return -1;
    }
}

int cwait(csem_t *sem){
    initializeCThread();

    if(sem == NULL){
        return -1;
    }

    sem->count = sem->count - 1;

    // If there is no recurse, sleep
    if(sem->count < 0){
        TCB_t* blockedThread = runningThread;
        blockedThread->state = PROCST_BLOQ;
        if(AppendFila2(&blockedQueue, (void*)blockedThread) != 0){
            return -1;
        }
        if(AppendFila2(sem->fila, (void*)blockedThread) != 0){
            return -1;
        }

        runningThread = NULL;

        //Call scheduler
        swapcontext(&(blockedThread->context), &scheduler);
    }

    return 0;
}

int csignal(csem_t *sem){
    initializeCThread();
    
    if(sem == NULL){
        return -1;
    }

    sem->count = sem->count + 1;

    // Se fila não está vazia
    if(FirstFila2(sem->fila) == 0){
        // Get first on semaphore queue and remove it
        TCB_t* wakeThread = (TCB_t*) GetAtIteratorFila2(sem->fila);
        DeleteAtIteratorFila2(sem->fila);
        
        // Remove from blocked and put on run queue
        if(wakeThread->state == PROCST_BLOQ){
            if(deleteTidOnQueue(&blockedQueue, wakeThread->tid) != 0){
                return -1;
            }
            if(AppendFila2(&runQueue, (void*)wakeThread) != 0){
                return -1;
            }
            wakeThread->state = PROCST_APTO;
        }
        // Remove from blocked suspend and put on run suspend queue
        else if(wakeThread->state == PROCST_BLOQ_SUS){
            if(deleteTidOnQueue(&sBlockedQueue, wakeThread->tid) != 0){
                return -1;
            }
            if(AppendFila2(&sRunQueue, (void*)wakeThread) != 0){
                return -1;
            }
            wakeThread->state = PROCST_APTO_SUS;
        }
        else{
            printf("Error: Thread was waiting at semaphore but wasn't neither blocked nor blocked suspend\n");
            return -1;
        }
    }

    return 0;
}
