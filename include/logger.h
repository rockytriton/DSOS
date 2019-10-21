#ifndef LOG_H
#define LOG_H

#include "common.h"
#include "util/str.h"

namespace dsos {

    class Logger {
    public:
        static Logger &inst();

        void print(const char *p);

        void println(const char *p) {
            print(p);
            print("\r\n");
        }

        Logger &operator<<(const char *p) {
            print(p);
        }

        Logger &operator<<(dword n) {
            String s;
            toString(n, s);
            print(s.c_str());
        }

        void printHex(dword n) {
            String s;
            toString(n, 16, true, s);
            print(s.c_str());
        }
    };
}

#endif