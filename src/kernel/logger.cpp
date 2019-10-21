#include "logger.h"
#include "../drivers/miniuart/miniuart.h"
#include "util/str.h"

using namespace dsos;

static Logger logger;
static bool loggerInit = false;

Logger &Logger::inst() {
    
    if (!loggerInit) {
        drivers::MiniUart::inst();
        loggerInit = true;
    }

    return logger;
}
void Logger::print(const char *p)  {
    dsos::drivers::MiniUart::inst()->send(p);
}


