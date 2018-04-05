#include <stdin.h>
#include <ucontext.h>
#include "cdata.h"
#include "support.h"
#include "cthread.h"

FILA2 runQueue;
FILA2 blockedQueue;
FILA2 sRunQueue;
FILA2 sBlockedQueue;

TCB_t *running;

ucontext_t scheduler;

int tid = 0;
int libraryInitialized = 0;

int getNextTid(){
	return ++tid;
}

void initializeCthread(){
	if(libraryInitialized)
		rerturn;

	if(CreateFila2(&runQueue) != 0)
		printf("Erro na criação da fila de aptos\n");
	if(CreateFila2(&srunQueue) != 0)
		printf("Erro na criação da fila de apto-suspenso\n");
	if(CreateFila2(&blockedQueue) != 0)
		printf("Erro na criação da fila de bloqueado\n");
	if(CreateFila2(&sBlockedQueue) != 0)
		printf("Erro na criação da fila de bloqueado-suspenso\n");
	
	TCB_t mainThread;
	mainThread.tid = 0;
	mainThread.prio = 0;
	mainThread.state = PROCST_EXEC;
	getcontext(&(mainThread.context));

	getcontext(&scheduler);
	makecontext(&scheduler, FUNCAOAQUI, 0);

	libraryInitialized = 1;
}

/******************************************************************************
Parâmetros:
	name:	ponteiro para uma área de memória onde deve ser escrito um string que contém os nomes dos componentes do grupo e seus números de cartão.
		Deve ser uma linha por componente.
	size:	quantidade máxima de caracteres que podem ser copiados para o string de identificação dos componentes do grupo.
Retorno:
	Se correto => 0 (zero)
	Se erro	   => Valor negativo.
******************************************************************************/
int cidentify (char *name, int size);

/******************************************************************************
Parâmetros:
	start:	ponteiro para a função que a thread executará.
	arg:	um parâmetro que pode ser passado para a thread na sua criação.
	prio:	NÃO utilizado neste semestre, deve ser sempre zero.
Retorno:
	Se correto => Valor positivo, que representa o identificador da thread criada
	Se erro	   => Valor negativo.
******************************************************************************/
int ccreate (void* (*start)(void*), void *arg, int prio){


	


}

/******************************************************************************
Parâmetros:
	Sem parâmetros
Retorno:
	Se correto => 0 (zero)
	Se erro	   => Valor negativo.
******************************************************************************/
int cyield(void);

/******************************************************************************
Parâmetros:
	tid:	identificador da thread cujo término está sendo aguardado.
Retorno:
	Se correto => 0 (zero)
	Se erro	   => Valor negativo.
******************************************************************************/
int cjoin(int tid);

/******************************************************************************
Parâmetros:
	tid:	identificador da thread a ser suspensa.
Retorno:
	Se correto => 0 (zero)
	Se erro	   => Valor negativo.
******************************************************************************/
int csuspend(int tid);

/******************************************************************************
Parâmetros:
	tid:	identificador da thread que terá sua execução retomada.
Retorno:
	Se correto => 0 (zero)
	Se erro	   => Valor negativo.
******************************************************************************/
int cresume(int tid);

/******************************************************************************
Parâmetros:
	sem:	ponteiro para uma variável do tipo csem_t. Aponta para uma estrutura de dados que representa a variável semáforo.
	count: valor a ser usado na inicialização do semáforo. Representa a quantidade de recursos controlados pelo semáforo.
Retorno:
	Se correto => 0 (zero)
	Se erro	   => Valor negativo.
******************************************************************************/
int csem_init(csem_t *sem, int count);

/******************************************************************************
Parâmetros:
	sem:	ponteiro para uma variável do tipo semáforo.
Retorno:
	Se correto => 0 (zero)
	Se erro	   => Valor negativo.
******************************************************************************/
int cwait(csem_t *sem);

/******************************************************************************
Parâmetros:
	sem:	ponteiro para uma variável do tipo semáforo.
Retorno:
	Se correto => 0 (zero)
	Se erro	   => Valor negativo.
******************************************************************************/
int csignal(csem_t *sem);
