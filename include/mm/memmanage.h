#pragma once

#include "mm.h"
#include "common.h"

namespace dsos {

    class MemManager {
    public:
        MemManager();
        static MemManager *inst() {
            return instance;
        }

        void *alloc(dword size);
        void free(void *p);

    private:
        static MemManager *instance;
    };
}