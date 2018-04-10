#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <string.h>
#include "cdata.h"
#include "support.h"
#include "cthread.h"

FILA2 runQueue;
FILA2 blockedQueue;
FILA2 sRunQueue;
FILA2 sBlockedQueue;

TCB_t mainThread;

TCB_t *runningThread;

ucontext_t scheduler;

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
    } while(NextFila2(queue) == 0);

    // Tid not found
    return -1;

}

void initializeCthread(){
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
    //makecontext(&scheduler, FUNCAOAQUI, 0);

    libraryInitialized = 1;
}

int cidentify(char *name, int size){

    char *group = "Lucas Nunes Alegre 00274693\nAline Weber\nLucas Sonntag Hagen\n\0";
    if(size < strlen(group)){
        printf("Size given is not sufficient to copy the whole string!\n");
        return -1;
    }

    strncpy(name, group, size-1);
    name[size] = '\0';

    return 0;
}

int ccreate (void* (*start)(void*), void *arg, int prio){
    initializeCthread();

    TCB_t* createdThread = (TCB_t*)malloc(sizeof(TCB_t));
    createdThread->tid = getNextTid();   
    createdThread->prio = 0;
    createdThread->state = PROCST_APTO;

    getcontext(&(createdThread->context));
    createdThread->context.uc_link = //linkarr;
    createdThread->context.uc_stack.ss_sp = (char*)malloc(SIGSTKSZ);
    createdThread->context.uc_stack.ss_size = SIGSTKSZ;
    makecontext(&(createdThread->context), (void(*)(void))start, 1, arg);

    if(AppendFila2(&runQueue, (void*)createdThread) != 0){
        printf("Error: insertion of the new thread in the Run Queue failed\n");
        return -1;
    }

    return 0;
}

int cyield(void){

    return 0;
}

int cjoin(int tid){
    
    return 0;
}

int csuspend(int tid) {
    initializeCthread();

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
    initializeCthread();

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
    initializeCthread();

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
    initializeCthread();

    if(sem == NULL){
        return -1;
    }

    sem->count = sem->count - 1;

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

        //chama escalonador
    }

    return 0;
}

int csignal(csem_t *sem){
    initializeCthread();
    
    if(sem == NULL){
        return -1;
    }

    sem->count = sem->count + 1;

    // Se fila n�o est� vazia
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
