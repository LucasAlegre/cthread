#include <stdio.h>
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

int tid = 0;
int libraryInitialized = 0;

int getNextTid(){
	return ++tid;
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
	makecontext(&scheduler, FUNCAOAQUI, 0);

	libraryInitialized = 1;
}

int cidentify (char *name, int size){

    char *group = "Lucas Nunes Alegre 00274693\nAline Weber\nLucas Sonntag Hagen\n\0";
    if(size < strlen(group){
        printf("Size given is not sufficient to copy the whole string!\n");
        return -1;
    }

    strncpy(name, group, size-1);
    name[size] = '\0';

    return 0;
}

int ccreate (void* (*start)(void*), void *arg, int prio){
    initializeCthread();
	
    TCB_t *createdThread = (*TCB_t)malloc(sizeof(TCB_t));
    createdThread->tid = getNextTid();   
    createdThread->prio = 0;
    createdThread->state = PROCST_APTO;
    
    getcontext(&(createdThread->context));
    createdThread->context.uc_link = //linkarr;
    createdThread->context.uc_stack.ss_sp = (char*)malloc(SIGSTKSZ);
    createdThread->context.uc_stack.ss_size = SIGSTKSZ;
    makecontext(&(createdThread->context), (void(*)void)start, 1, arg);

    if(AppendFila2(&runQueue, (void*)createdThread) != 0){
        printf("Error: insertion of the new thread in the Run Queue failed\n");
        return -1;
    }

    return 0;    
}

int cyield(void);

int cjoin(int tid);

int csuspend(int tid);

int cresume(int tid);

int csem_init(csem_t *sem, int count);

int cwait(csem_t *sem);

int csignal(csem_t *sem);
