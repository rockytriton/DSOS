#ifndef STR_H
#define STR_H

#include "common.h"

namespace dsos {

    class String;

    dword stringLen(const char *p);

    dword copyString(char *dest, const char *source);

    void toString(dword num, byte base, bool upper, String &s);

    void toString(signed int num, String &s);

    class String {
    public:
        String() : String("") {}

        String(const char *p) {
            setString(p);
        }

        ~String() {
            delete[] s;
        }

        String &operator=(const char *p) {
            setString(p);
        }

        String &operator+=(const char *p) {
            char *tmp = new char[size(p) + len + 1];
            copyString(tmp, s);
            copyString(tmp + len, p);
            setString(tmp);
            delete[] tmp;
        }

        int size() { return len; }

        const char *c_str() {
            return (const char *)s;
        }

        static int size(const char *p);

    private:
        void setString(const char *p) {
            //if (s != nullptr) {
            //    delete[] s;
            //}

            len = size(p);
            s = new char[len + 1];
            copyString(s, p);
        }

        int len = 0;
        char *s = nullptr;
    };



}

#endif