#pragma once

#include "common.h"

namespace dsos {
namespace mailbox {
#define PROPTAG_SET_POWER_STATE		0x00028001
#define PROPTAG_GET_CLOCK_RATE		0x00030002

    struct Tag {
        dword tagId;
        dword bufferSize;
        dword valueLength;
    };

    class Mailbox {
    public:
        static Mailbox *inst();
        
        bool processTag(dword tagId, Tag &tag, dword tagSize);

    private:
    };

}
}