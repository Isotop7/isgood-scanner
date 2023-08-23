#ifndef COMMAND_H
#define COMMAND_H

#include <Arduino.h>

class Command
{
public:
    Command();
    Command(u_int optionCode, u_int value);

    String getOptionCode() const;
    u_int getValue() const;

    static Command parse(const char *data);

    static const String Suffix;

    class Mode
    {
    public:
        static const String MODIFY;
        static const String QUERY;
    };

    enum class Response : char
    {
        INVALID = 0,
        ACK = 6,
        ENQ = 5,
        EOL = '\n'
    };

private:
    u_int _optionCode;
    u_int _value;
};

#endif