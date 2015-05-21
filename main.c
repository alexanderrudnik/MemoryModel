#include <stdio.h>
#include <stdlib.h>
#include "mmemory.h"

#define PAGE_NUMBER 16
#define PAGE_SIZE 128

VA doMalloc(void);
void doWrite(void);
void doRead(void);
void doFree(void);

int main(void)
{
    int choice = -1;
    bool runnable = true;
    printf("Init code: %i\n", _init_memory(PAGE_NUMBER, PAGE_SIZE));
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
            runnable = false;
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

