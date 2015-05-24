#include "mmemory.h"
#include "stdio.h"

#define SWAP_FILE "swap.dat"
#define MEMORY_SIZE 512

enum ReturnCode {
    SUCCESS = 0,
    UNKNOWN_ERROR = 1,
    WRONG_PARAMETERS = -1,
    NOT_ENOUGH_SPACE = -2,
    OUT_OF_BLOCK = -2,
    FILE_NOT_FOUND = -3,
    END_OF_FILE = -4
};

struct Block {
    struct Block *pNext;
    unsigned int szBlock;
    unsigned long offsetBlock;
};

struct PageInfo {
    unsigned long offsetPage;
    bool isUse;
};

struct Page {
    struct Block *pFirstFree;
    struct Block *pFirstUse;
    unsigned int freeBlockSize;
    struct PageInfo pInfo;
};

VA memory; //physical memory

struct Page *pageTable;

int pageSize;
int pageNumb;

int _init_memory(int n, int szPage);
int _malloc(VA *ptr, size_t szBlock);
int _write(VA ptr, size_t delt, void *pBuffer, size_t szBuffer);
int _read(VA ptr, size_t delt, void *pBuffer, size_t szBuffer);
int _free(VA ptr);

void printInfo();
int initPageTable();
int writeToFile(int offset, VA page);


int _init_memory (int n, int szPage) {
    if (n <= 0 || szPage <= 0) {
        return WRONG_PARAMETERS;
    }
    remove(SWAP_FILE);

    pageSize = szPage;
    pageNumb = n;
    pageTable = (struct Page*) malloc(sizeof(struct Page) * pageNumb);
    memory = (VA) malloc(sizeof(char) * MEMORY_SIZE);

    printf("%d", *memory);

    if (NULL == pageTable || NULL == memory) {
        return UNKNOWN_ERROR;
    }

    return initPageTable();
}

int initPageTable() {
    int memoryOffsetPage = 0;
    int swapOffsetPage = 0;

    for (int i = 0; i < pageNumb; ++i) {
        struct Block *block = (struct Block*) malloc(sizeof(struct Block));
        block -> pNext = NULL;
        block -> szBlock = pageSize;
        block -> offsetBlock = 0;

        pageTable[i].pFirstFree = block;
        pageTable[i].pFirstUse = NULL;
        pageTable[i].freeBlockSize = pageSize;

        if ((i + 1) * pageSize > MEMORY_SIZE) {
            //write to swap file
            if(!writeToFile(swapOffsetPage, memory)) {
                pageTable[i].pInfo.offsetPage = swapOffsetPage;
                pageTable[i].pInfo.isUse = false;
            } else {
                return UNKNOWN_ERROR;
            }
        } else {
            pageTable[i].pInfo.offsetPage = memoryOffsetPage;
            pageTable[i].pInfo.isUse = true;
        }

    }
    return SUCCESS;
}


int _malloc (VA* ptr, size_t szBlock) {
    if (NULL == ptr || szBlock <= 0) {
        return WRONG_PARAMETERS;
    }
    return SUCCESS;
}

int _write (VA ptr, size_t delt, void* pBuffer, size_t szBuffer) {
    if (NULL == ptr || delt <= 0 || NULL == pBuffer || szBuffer <= 0) {
        return WRONG_PARAMETERS;
    }
    return SUCCESS;
}

int _read (VA ptr, size_t delt, void* pBuffer, size_t szBuffer) {
    if (NULL == ptr || delt <= 0 || NULL == pBuffer || szBuffer <= 0) {
        return WRONG_PARAMETERS;
    }
    return SUCCESS;
}

int _free (VA ptr) {
    if (NULL == ptr) {
        return WRONG_PARAMETERS;
    }
    return SUCCESS;
}

int writeToFile(int offset, VA page) {
    //open file for read
    FILE *file = fopen(SWAP_FILE, "r+");

    if (NULL == file) {
        //if file doesn't exist, create it
        file = fopen(SWAP_FILE, "w");
        if (NULL == file) {
            return UNKNOWN_ERROR;
        }
    }
    //cдвинуть положение указателя на offset и проверить на правильность
    if (0 != fseek(file, offset, SEEK_SET)) {
        return END_OF_FILE;
    }
    for (int i = 0; i < pageSize; ++i) {
        fputc(page[i], file); //why ??????
    }
    printInfo();
    fclose(file);
    return SUCCESS;
}

void printInfo() {
    for (int i = 0; i < MEMORY_SIZE; ++i) {
        printf("%c", memory[i]);
    }
    printf("\n");
}
