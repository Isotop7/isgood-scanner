#ifndef COMMANDFORMAT_H
#define COMMANDFORMAT_H

#include <Arduino.h>

class CommandFormat {
    public:
        static const String SetPrefix;
        static const String QueryPrefix;
        static const String Suffix;
};

#endif