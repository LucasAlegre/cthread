#include <stdio.h>
#include <stdlib.h>

#include "cthread.h"

void func1();

int th1;

void func1() {
	printf("[%d]: START\n", 1);
	printf("[%d]: STOP\n", 1);
}

int main(){
	th1 = ccreate((void*) func1, (void*) NULL, 0);
	printf("[MAIN] Thread with pid = %d\n", th1);

	printf("[MAIN] CSUSPEND %d\n", th1);
	csuspend(th1);
	cjoin(th1);

	printf("[MAIN] STOPPED\n");
	return 0;
}
