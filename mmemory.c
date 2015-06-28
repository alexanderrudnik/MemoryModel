#include "mmemory.h"
#include "stdio.h"

#define PAGING_FILE "paging_file.dat"
#define MEMORY_SIZE 512


enum ReturnCode {
    SUCCESS = 0,
    UNKNOWN_ERROR = 1,
    WRONG_PARAMETERS = -1,
    NOT_ENOUGH_SPACE = -2,
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
    unsigned int activity;
};

VA physicalMemory;
struct Page *pageTable;

unsigned int pageSize;
unsigned int pageNumb;

int _init_memory(int n, int szPage);
int _malloc(VA *ptr, size_t szBlock);
int _write(VA ptr, void *pBuffer, size_t szBuffer);
int _read(VA ptr, void *pBuffer, size_t szBuffer);
int _free(VA ptr);

void printInfo();
int writeToFile(unsigned int offset, VA writtenPage);
int readFromFile(unsigned int offset, VA readPage);
int mallocInOnePage(VA* ptr, size_t szBlock, int numb);
void addToUsedBlocks(struct Page* page, struct Block* block);
VA convertVAtoPA(VA ptr, unsigned int *offsetPage, unsigned int *offsetBlock);
void swapPages(struct Page* fromMemory, struct Page* inMemory);
struct Page* getNonActivePage();
unsigned int getMaxSizeFreeBlock(struct Page page);
struct Block* getRequiredBlock(unsigned int offsetPage, unsigned int offsetBlock);
struct Block* getPreviousBlock(unsigned int offsetPage, unsigned int offsetBlock);
void addToFreeBlock(struct Page* page, struct Block* block);


int _init_memory (int n, int szPage) {
    if (n <= 0 || szPage <= 0) {
        return WRONG_PARAMETERS;
    }
    remove(PAGING_FILE);

    pageSize = szPage;
    pageNumb = n;
    pageTable = (struct Page*) malloc(sizeof(struct Page) * pageNumb);
    physicalMemory = (VA) malloc(sizeof(char) * MEMORY_SIZE);

    if (NULL == pageTable || NULL == physicalMemory) {
        return UNKNOWN_ERROR;
    }

    int memoryOffsetPage = 0;
    int swapOffsetPage = 0;

    for (unsigned int i = 0; i < pageNumb; ++i) {
        struct Block *block = (struct Block*) malloc(sizeof(struct Block));
        block -> pNext = NULL;
        block -> szBlock = pageSize;
        block -> offsetBlock = 0;

        pageTable[i].pFirstFree = block;
        pageTable[i].pFirstUse = NULL;
        pageTable[i].freeBlockSize = pageSize;
        pageTable[i].activity = 0;

        if ((i + 1) * pageSize > MEMORY_SIZE) {
            if(!writeToFile(swapOffsetPage, physicalMemory)) {
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
    struct Page* reservePage = NULL;
    int reservePageIndex = -1;

    for (unsigned int i = 0; i < pageNumb; ++i) {
        if (pageTable[i].freeBlockSize >= szBlock) {
            if (pageTable[i].pInfo.isUse) {
                return mallocInOnePage(ptr, szBlock, i);
            }
            else if (NULL == reservePage) {
                reservePage = &pageTable[i];
                reservePageIndex = i;
            }
        }
    }

    if (NULL != reservePage) {
        swapPages(getNonActivePage(), reservePage);
        return mallocInOnePage(ptr, szBlock, reservePageIndex);
    }

    return NOT_ENOUGH_SPACE;
}

int _write (VA ptr, void* pBuffer, size_t szBuffer) {
    if (NULL == pBuffer || szBuffer <= 0) {
        return WRONG_PARAMETERS;
    }

    VA vaddr;
    unsigned int offsetPage, offsetBlock;
    vaddr = convertVAtoPA(ptr, &offsetPage, &offsetBlock);
    printf("%s", vaddr);
    if (!vaddr) {
        swapPages(getNonActivePage(), &pageTable[offsetPage]);
        vaddr = convertVAtoPA(ptr, &offsetPage, &offsetBlock);
    }

    struct Block* writtenBlock = getRequiredBlock(offsetPage, offsetBlock);
    if (szBuffer > writtenBlock -> szBlock) {
        return NOT_ENOUGH_SPACE;
    }
    char* paBuffer = (char*) pBuffer;
    for (unsigned int i = 0; i < szBuffer; ++i) {
        vaddr[i] = paBuffer[i];
    }
    pageTable[offsetPage].activity++;
    return SUCCESS;
}

int _read (VA ptr, void* pBuffer, size_t szBuffer) {
    if (NULL == pBuffer || szBuffer <= 0) {
        return WRONG_PARAMETERS;
    }

    VA raddr;
    unsigned int offsetPage, offsetBlock;
    raddr = convertVAtoPA(ptr, &offsetPage, &offsetBlock);
    if (NULL == raddr) {
        swapPages(getNonActivePage(), &pageTable[offsetPage]);
        raddr = convertVAtoPA(ptr, &offsetPage, &offsetBlock);
    }

    struct Block* readBlock = getRequiredBlock(offsetPage, offsetBlock);
    if (szBuffer > readBlock -> szBlock) {
        return NOT_ENOUGH_SPACE;
    }
    char* vaBuffer = (char*) pBuffer;
    for (unsigned int i = 0; i < szBuffer; ++i) {
        vaBuffer[i] = raddr[i];
    }
    pageTable[offsetPage].activity++;
    return SUCCESS;
}

int _free (VA ptr) {
    unsigned int offsetPage, offsetBlock;
    convertVAtoPA(ptr, &offsetPage, &offsetBlock);
    printf("\n%i %i\n", offsetPage, offsetBlock);
    struct Block* freeBlock = getRequiredBlock(offsetPage, offsetBlock);
    struct Block* prevBlock = getPreviousBlock(offsetPage, offsetBlock);

    if (NULL != prevBlock) {
        prevBlock -> pNext = freeBlock -> pNext;
    }
    else {
        pageTable[offsetPage].pFirstUse = freeBlock -> pNext;
    }
    addToFreeBlock(&pageTable[offsetPage], freeBlock);
    return SUCCESS;
}

int writeToFile(unsigned int offset, VA writtenPage) {
    FILE *file = fopen(PAGING_FILE, "r+");
    unsigned int offsetFile = offset * pageSize;

    if (NULL == file) {
        file = fopen(PAGING_FILE, "w");
        if (NULL == file) {
            fclose(file);
            return UNKNOWN_ERROR;
        }
    }
    if (0 != fseek(file, offsetFile, SEEK_SET)) {
        fclose(file);
        return END_OF_FILE;
    }
    for (unsigned int i = 0; i < pageSize; ++i) {
        fputc(writtenPage[i], file);
    }
    fclose(file);
    return SUCCESS;
}

int readFromFile(unsigned int offset, VA readPage) {
    FILE *file = fopen(PAGING_FILE, "r");
    unsigned int offsetFile = offset * pageSize;

    if (NULL == file) {
        fclose(file);
        return FILE_NOT_FOUND;
    }
    if (0 != fseek(file, offsetFile, SEEK_SET)) {
        fclose(file);
        return END_OF_FILE;
    }
    for (unsigned int i = 0; i < pageSize; ++i) {
        readPage[i] = fgetc(file);
    }
    fclose(file);
    return SUCCESS;
}

int mallocInOnePage(VA* ptr, size_t szBlock, int numb) {
    struct Page* page = &pageTable[numb];
    struct Block* blockPtr = page -> pFirstFree;
    struct Block* parentBlock = NULL;

    while(blockPtr) {
        if (blockPtr -> szBlock >= szBlock) {
            int virtualAddress = numb * pageSize + blockPtr -> offsetBlock;
            *ptr = (VA) virtualAddress;
            printf("\nptr=%i\n", (int)*ptr);

            if (blockPtr -> szBlock == szBlock) {
                if (parentBlock) {
                    parentBlock -> pNext = blockPtr -> pNext;
                }
                else {
                    page -> pFirstFree = NULL;
                    blockPtr -> pNext = NULL;
                    addToUsedBlocks(page, blockPtr);
                }
            }
            else {
                struct Block* usedBlock;
                usedBlock = (struct Block*) malloc(sizeof(struct Block));
                usedBlock -> offsetBlock = blockPtr -> offsetBlock;
                usedBlock -> pNext = NULL;
                usedBlock -> szBlock = szBlock;
                addToUsedBlocks(page, usedBlock);

                blockPtr -> szBlock -= szBlock;
                blockPtr -> offsetBlock += szBlock;
            }
            page -> activity++;
            page -> freeBlockSize = getMaxSizeFreeBlock(*page);
            return SUCCESS;
        }
        else {
            parentBlock = blockPtr;
            blockPtr = blockPtr -> pNext;
        }
    }
    return UNKNOWN_ERROR;
}

unsigned int getMaxSizeFreeBlock(struct Page page) {
    unsigned int maxSize = 0;
    struct Block *block = page.pFirstFree;

    while(NULL != block) {
        if (block -> szBlock > maxSize) {
            maxSize = block -> szBlock;
        }
        block = block -> pNext;
    }
    return maxSize;
}

void addToUsedBlocks(struct Page* page, struct Block* block) {
    struct Block* blockPtr = page -> pFirstUse;
    struct Block* prevBlockPtr = NULL;

    while(blockPtr) {
        prevBlockPtr = blockPtr;
        blockPtr = blockPtr -> pNext;
    }
    if (prevBlockPtr) {
        prevBlockPtr -> pNext = block;
        block -> pNext = NULL;
    }
    else {
        page -> pFirstUse = block;
    }
}

void addToFreeBlock(struct Page* page, struct Block* block) {
    struct Block* blockPtr = page -> pFirstFree;
    struct Block* prevBlockPtr = NULL;

    while (blockPtr) {
        prevBlockPtr = blockPtr;
        blockPtr = blockPtr -> pNext;
    }

    if (prevBlockPtr) {
        if (prevBlockPtr -> offsetBlock + prevBlockPtr -> szBlock == block -> offsetBlock) {
            prevBlockPtr -> szBlock += block -> szBlock;
            free(block);
        }
        else {
            prevBlockPtr -> pNext = block;
            block -> pNext = NULL;
        }
    }
    else {
        page -> pFirstFree = block;
    }
}

VA convertVAtoPA(VA ptr, unsigned int *offsetPage, unsigned int *offsetBlock) {
    int address = (int) ptr;
    *offsetPage = address / pageSize;
    *offsetBlock = address % pageSize;
    if (pageTable[*offsetPage].pInfo.isUse) {
        return physicalMemory + pageTable[*offsetPage].pInfo.offsetPage * pageSize + *offsetBlock;
    }
    return NULL;
}

void swapPages(struct Page *fromMemory, struct Page *inMemory) {
    struct PageInfo bufInfo;
    VA memoryPtr = physicalMemory + fromMemory -> pInfo.offsetPage * pageSize;
    VA memoryPageContent = (VA) malloc(sizeof(char) * pageSize);

    for (int i = 0; i < pageSize; ++i) {
        memoryPageContent[i] = memoryPtr[i];
    }
    readFromFile(inMemory -> pInfo.offsetPage, memoryPtr);
    writeToFile(inMemory -> pInfo.offsetPage, memoryPageContent);

    bufInfo = fromMemory -> pInfo;
    fromMemory -> pInfo = inMemory -> pInfo;
    inMemory -> pInfo = bufInfo;
    inMemory -> activity++;
}

struct Page* getNonActivePage() {
    unsigned int minActivity = pageTable[0].activity;
    struct Page* minActivePage = &pageTable[0];
    for (unsigned int i = 1; i < pageNumb; ++i) {
        if (pageTable[i].pInfo.isUse && pageTable[i].activity < minActivity) {
            minActivity = pageTable[i].activity;
            minActivePage = &pageTable[i];
        }
    }
    return minActivePage;
}

struct Block* getRequiredBlock(unsigned int offsetPage, unsigned int offsetBlock) {
    struct Block* tempBlock = pageTable[offsetPage].pFirstUse;
    while(NULL != tempBlock && tempBlock -> offsetBlock != offsetBlock) {
        tempBlock = tempBlock -> pNext;
    }
    return tempBlock;
}

struct Block* getPreviousBlock(unsigned int offsetPage, unsigned int offsetBlock) {
    struct Block* prevBlock = NULL;
    struct Block* nextBlock = pageTable[offsetPage].pFirstUse;
    while (NULL != nextBlock && nextBlock -> offsetBlock != offsetBlock) {
        prevBlock = nextBlock;
        nextBlock = nextBlock -> pNext;
    }
    return prevBlock;
}

void printInfo() {
    for (int i = 0; i < MEMORY_SIZE; ++i) {
        printf("%c", physicalMemory[i]);
    }
    printf("\n");
}
