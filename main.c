#include <stdio.h>
#include <stdlib.h>
#include "mmemory.h"

VA doMalloc(void);
void doWrite(void);
void doRead(void);
void doFree(void);

int main(void)
{
    int choice, runnable = 1;
    while(runnable) {
        printf("============================");
        printf("\n1 - Malloc\n2 - Write\n3 - Read\n4 - Free\n");
        printf("----------------------------");
        printf("\n5 - Exit\n");
        printf("============================");
        printf("\nYour choice - ");
        scanf("%i", &choice);
        switch(choice) {
        case 1:
            doMalloc();
            break;
        case 2:
            doWrite();
            break;
        case 3:
            doRead();
            break;
        case 4:
            doFree();
            break;
        case 5:
            runnable = 0;
            printf("See you later\n");
            break;
        default:
            break;
        }
    }
    return 0;
}

VA doMalloc() {
    VA a;
    return a;
}

void doWrite() {

}

void doRead() {

}

void doFree() {

}

