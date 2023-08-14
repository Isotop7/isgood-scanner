
#include "MHET_Live_Barcode_Scanner.h"
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <vector>
#include "Command.h"

SoftwareSerial _serial;

MHET_Live_Barcode_Scanner::MHET_Live_Barcode_Scanner(SoftwareSerial* serial, unsigned long timeout) 
{
    _serial = serial;
    _timeout = timeout;
}

void MHET_Live_Barcode_Scanner::resetSettings()
{
    configureOption(OptionCodes::getHexValueForEnum(OptionCodes::OptionCode::SetupMode), OptionCodes::Values::SetupMode::ACTIVE);
    configureOption(OptionCodes::getHexValueForEnum(OptionCodes::OptionCode::DeviceSettings), 0x1B2C);
    configureOption(OptionCodes::getHexValueForEnum(OptionCodes::OptionCode::DeviceSettings), 0xD201);
    configureOption(OptionCodes::getHexValueForEnum(OptionCodes::OptionCode::ResetOutput), 0);
    configureOption(OptionCodes::getHexValueForEnum(OptionCodes::OptionCode::DeviceSettings), 0x506A);
    configureOption(OptionCodes::getHexValueForEnum(OptionCodes::OptionCode::SetupMode), OptionCodes::Values::SetupMode::INACTIVE);
}

void MHET_Live_Barcode_Scanner::enableOutput()
{
    configureOption(OptionCodes::getHexValueForEnum(OptionCodes::OptionCode::SetupMode), OptionCodes::Values::SetupMode::ACTIVE);
    configureOption(OptionCodes::getHexValueForEnum(OptionCodes::OptionCode::ResetOutput), 0);
    configureOption(OptionCodes::getHexValueForEnum(OptionCodes::OptionCode::DeviceSettings), 0x506A);
    configureOption(OptionCodes::getHexValueForEnum(OptionCodes::OptionCode::SetupMode), OptionCodes::Values::SetupMode::INACTIVE);
}

Command::Response MHET_Live_Barcode_Scanner::configureOption(int optionCode, int value)
{
    String command = Command::Mode::MODIFY;

    String optionCodeStr = String(optionCode, HEX);
    if (optionCodeStr.length() < 4) {
        while (optionCodeStr.length() < 4)
        {
            optionCodeStr = "0" + optionCodeStr;
        }
    }
    String valueStr = String(value, HEX);
    if (valueStr.length() < 4) {
        while (valueStr.length() < 4)
        {
            valueStr = "0" + valueStr;
        }
    }

    command += optionCodeStr;
    command += valueStr;
    command += Command::Suffix;

    _serial->write(command.c_str());
    delay(_timeout);

    if (_serial->available())
    {
        char response = _serial->read();

        if (response == static_cast<char>(Command::Response::ACK))
        {
            return Command::Response::ACK;
        }
        else if (response == static_cast<char>(Command::Response::ENQ))
        {
            return Command::Response::ENQ;
        }
        else
        {
            return Command::Response::INVALID;
        }
    }
    return Command::Response::INVALID;
}

void MHET_Live_Barcode_Scanner::setOption(int optionCode, int value)
{
    String command = Command::Mode::MODIFY;

    String optionCodeStr = String(optionCode, HEX);
    if (optionCodeStr.length() < 4) {
        while (optionCodeStr.length() < 4)
        {
            optionCodeStr = "0" + optionCodeStr;
        }
    }
    String valueStr = String(value, HEX);
    if (valueStr.length() < 4) {
        while (valueStr.length() < 4)
        {
            valueStr = "0" + valueStr;
        }
    }

    command += optionCodeStr;
    command += valueStr;
    command += Command::Suffix;

    _serial->write(command.c_str());
    delay(_timeout);
}

void MHET_Live_Barcode_Scanner::getOption(int optionCode)
{
    String command = Command::Mode::QUERY;

    String optionCodeStr = String(optionCode, HEX);
    if (optionCodeStr.length() < 4) {
        while (optionCodeStr.length() < 4)
        {
            optionCodeStr = "0" + optionCodeStr;
        }
    }

    command += optionCodeStr;
    command += Command::Suffix;

    _serial->write(command.c_str());
    delay(_timeout);
}

Command::Response MHET_Live_Barcode_Scanner::queryConfiguration()
{
    String config;
    // Init config mode
    configureOption(OptionCodes::getHexValueForEnum(OptionCodes::OptionCode::SetupMode), OptionCodes::Values::SetupMode::ACTIVE);
    // Query config
    getOption(OptionCodes::getHexValueForEnum(OptionCodes::OptionCode::ConfigurationManagement));
    // Wait for data to arrive in buffer
    delay(_timeout);
    
    // Read data from buffer
    while (_serial->available())
    {
        char key = _serial->read();
        // Add key to config
        config += key;

        // If we cant find data, wait for the serial timeout to be sure no more data has arrived while emptying buffer
        if (_serial->available() == 0)
        {
            delay(_timeout);
        }
    }

    configureOption(OptionCodes::getHexValueForEnum(OptionCodes::OptionCode::SetupMode), OptionCodes::Values::SetupMode::INACTIVE);

    int configSize = config.length();
    char rawConfig[configSize];
    config.toCharArray(rawConfig, configSize, 1);
    readConfig(rawConfig, configSize);

    return Command::Response::ACK;
}

void MHET_Live_Barcode_Scanner::readConfig(const char* rawConfig, size_t length)
{
    const char* lineStart = rawConfig;
    const char* lineEnd;

    // Iterate through the input string to process each line
    while (*lineStart != '\0') {
        // Find the end of the current line
        lineEnd = strchr(lineStart, '\n');
        if (lineEnd == nullptr) {
            // If there's no newline character, consider the rest of the string as the line
            lineEnd = lineStart + strlen(lineStart);
        }

        // Extract the current line into a temporary buffer
        char buffer[50];  // Adjust the buffer size as needed
        size_t lineLength = lineEnd - lineStart;
        strncpy(buffer, lineStart, lineLength);
        buffer[lineLength] = '\0';

        // Parse the line and create a custom instance using the "parse" method
        Command command = Command::parse(buffer);

        // Add the parsed instance to the vector
        _configuration.push_back(command);

        // Move to the next line
        lineStart = lineEnd + 1;
    }
}

std::vector<Command> MHET_Live_Barcode_Scanner::getConfigurationInstance() const
{
    return _configuration;
}

String MHET_Live_Barcode_Scanner::getConfigurationAsString() const
{
    String config = "{\r\n";    
    for (const Command& obj : _configuration) {
        config += "\t\"";
        config += obj.getOptionCode();
        config += "\": ";
        config += String(obj.getValue(), HEX);
        config += ",\r\n";
    }
    config += "}";
    return config;
}

String MHET_Live_Barcode_Scanner::getConfigurationAsHTML() const
{
    String config = "<table>\r\n";
    config += "\t<tr>\r\n";
    config += "\t\t<th>OptionCode</th>\r\n";
    config += "\t\t<th>Value</th>\r\n";
    config += "\t</tr>\r\n";
    for (const Command& obj : _configuration) {
        config += "\t<tr>\r\n";
        config += "\t\t<td>";
        config += obj.getOptionCode();
        config += "\t\t</td>\r\n";
        config += "\t\t<td>";
        config += String(obj.getValue(), HEX);
        config += "\t\t</td>\r\n";
        config += "\t</tr>\r\n";
    }
    config += "</table>\r\n";
    return config;
}

String MHET_Live_Barcode_Scanner::getNextBarcode() const
{
  String barcode = "";
  char receivedChar;

  while (_serial->available())
  {
    receivedChar = _serial->read();
    if (receivedChar == static_cast<char>(Command::Response::EOL)) {
      break; // Exit the loop if line feed character is received
    }
    barcode += receivedChar;

    // If we cant find data, wait for the serial timeout to be sure no more data has arrived while emptying buffer
    if (_serial->available() == 0)
    {
      delay(_timeout);
    }
  }
  barcode.trim();
  return barcode;
}