#include <stdio.h>
#include <stdlib.h>
#include "mmemory.h"

#define PAGE_NUMBER 16
#define PAGE_SIZE 64

void doMalloc(void);
void doWrite(void);
void doRead(void);
void doFree(void);

VA ptr;

int main(void)
{
    int choice = -1;
    bool runnable = true;
    printf("Init code = %i\n", _init_memory(PAGE_NUMBER, PAGE_SIZE));
    while(runnable) {
        printf("\n============================");
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

void doMalloc() {
    printf("\nMalloc\nInput memory size - ");
    unsigned int memorySize;
    scanf("%i", &memorySize);
    printf("Return code = %i\n", _malloc(&ptr, memorySize));
}

void doWrite() {
    printf("\nWrite\nInput address - ");
    int address;
    scanf("%i", &address);
    ptr = (VA) address;
    char buffer[1000];
    printf("\nInput data - ");
    scanf("%s", &buffer);
    printf("Return code = %i\n", _write(ptr, buffer, strlen(buffer)));
}

void doRead() {
    printf("\nRead\nInput address - ");
    int address;
    scanf("%i", &address);
    ptr = (VA) address;
    char buffer[1000];
    printf("\nInput size - ");
    unsigned int bufferSize;
    scanf("%u", &bufferSize);
    printf("Return code = %i\n", _read(ptr, &buffer, bufferSize + 1));
    printf("%s", buffer);
}

void doFree() {
    printf("\nFree\nInput address - ");
    int address;
    scanf("%i", &address);
    ptr = (VA) address;
    printf("Return code = %i\n", _free(ptr));
    _write(ptr, " ", 1);
}

