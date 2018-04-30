#include <stdio.h>
#include <stdlib.h>

#include "cthread.h"

void func1();
void func2();

int th1, th2;

void func1() {
	printf("[%d]: START\n", th1);

	cjoin(th2);

	printf("[%d]: STOP\n", th1);
}

void func2() {
	printf("[%d]: START\n", th2);

	csuspend(th1);

	printf("[%d]: STOP\n", th2);
}

int main(){
	th1 = ccreate((void*) func1, (void*) NULL, 0);
	printf("[MAIN] Thread with pid = %d\n", th1);

	th2 = ccreate((void*) func2, (void*) NULL, 0);
	printf("[MAIN] Thread with pid = %d\n", th2);

	printf("[MAIN] CYIELD\n");
	cyield();
	
	printf("[MAIN] CRESUME %d\n", th1);
	cresume(th1);

	printf("[MAIN] CJOIN %d\n", th1);
	cjoin(th1);

	printf("[MAIN] STOPPED\n");
	return 0;
}
