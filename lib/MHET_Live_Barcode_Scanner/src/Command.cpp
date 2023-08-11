#include <Arduino.h>
#include "Command.h"
#include "OptionCode.h"

Command::Command()
{
    _optionCode = 0;
    _value = 0;
}

Command::Command(u_int optionCode, u_int value)
{
    _optionCode = optionCode;
    _value = value;
}

String Command::getOptionCode() const {
    OptionCodes::OptionCode optionCode = OptionCodes::getEnumForHexValue(_optionCode);
    const std::map<OptionCodes::OptionCode, String>& map = OptionCodes::getOptionCodeMap();
    auto it = map.find(optionCode);
    if (it != map.end()) {
        return it->second;
    } else {
        char formattedString[5];  // Adjust the buffer size as needed
        sprintf(formattedString, "0x%04X", _optionCode);
        return formattedString;
    }
}

u_int Command::getValue() const {
    return _value;
}

Command Command::parse(const char* data)
{
    char optionCodeStr[5];
    char valueStr[5];
    int optionCode, value;

    int count = sscanf(data, "%4s\t%4s\n", optionCodeStr, valueStr);
    if (count == 2) {
        optionCode = strtol(optionCodeStr, nullptr, 16);
        value = strtol(valueStr, nullptr, 16);
        Command command(optionCode, value);
        return command;
    }

    return Command();
}