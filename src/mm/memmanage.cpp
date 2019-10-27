#include "mm/memmanage.h"

#include "logger.h"
#include "printf.h"

using namespace dsos;

struct MemPage {
    bool used;
    dword size;
};

const dword RAM_PAGE_SIZE = 256;
const dword TOTAL_RAM = (PAGE_SIZE * PAGING_PAGES);
const dword MM_BASE = LOW_MEMORY + (PAGE_SIZE * PAGING_PAGES);
const dword PAGE_TABLE_SIZE = (TOTAL_RAM / RAM_PAGE_SIZE) * sizeof(MemPage);
const dword RAM_START = MM_BASE + PAGE_TABLE_SIZE;

static MemPage *page_table = (MemPage *)MM_BASE;

static MemManager mm;
MemManager *MemManager::instance = &mm;

static bool meminit = false;

MemManager::MemManager() {
    if (this != &mm) {
        *(char *)0 = 1;
    }

    for (int i=0; i<PAGE_TABLE_SIZE; i++) {
        page_table[i] = {false, 0};
    }
}

void *MemManager::alloc(dword size) {
    if (!meminit) {
        meminit = true;
        memzero((unsigned long)page_table, PAGE_TABLE_SIZE * sizeof(MemPage));

        //for (int i=0; i<PAGE_TABLE_SIZE; i++) {
        //    page_table[i] = {false, 0};
        //}
    }
    
    dword numPages = size <= RAM_PAGE_SIZE ? 1 : (size / RAM_PAGE_SIZE) + 1;

    for (int i=0; i<PAGE_TABLE_SIZE; i++) {
        if (page_table[i].used) {
            continue;
        }

        bool pageFound = true;
        for (int p=i; p<i + numPages; p++) {
            if (page_table[p].used) {
                pageFound = false;
                break;
            }
        }

        if (pageFound) {
            for (int p=i; p<i + numPages; p++) {
                page_table[p].used = true;
                page_table[p].size = numPages;
            }

            return (void *)(RAM_START + (RAM_PAGE_SIZE * i));
        }
    }

    return nullptr;
}

void MemManager::free(void *p) {

    dword pos = (dword)(uint64_t)p;
    dword page = (pos - (RAM_START)) / RAM_PAGE_SIZE;
    dword numPages = page_table[page].size;

    for (dword i=0; i<numPages; i++) {
        page_table[i + page].used = false;
        page_table[i + page].size = 0;
    }

}



int lastAllocSize = 0;

void* operator new[](unsigned long size) { lastAllocSize = size; return mm.alloc(size); }
void* operator new(unsigned long size) { lastAllocSize = size; return mm.alloc(size); }
void operator delete(void* ptr) { mm.free(ptr); }
void operator delete[](void* ptr) { mm.free(ptr); }
