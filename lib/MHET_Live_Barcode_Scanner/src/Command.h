#ifndef COMMAND_H
#define COMMAND_H

#include <Arduino.h>

class Command {
    public:
        Command();
        Command(u_int optionCode, u_int value);

        String getOptionCode() const;
        u_int getValue() const;

        static Command parse(const char* data);
    private:
        u_int _optionCode;
        u_int _value;
};

#endif