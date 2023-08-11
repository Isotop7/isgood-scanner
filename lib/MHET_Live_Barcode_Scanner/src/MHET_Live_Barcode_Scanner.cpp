
#include "MHET_Live_Barcode_Scanner.h"
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <vector>
#include "CommandFormat.h"
#include "Command.h"

SoftwareSerial _serial;

MHET_Live_Barcode_Scanner::MHET_Live_Barcode_Scanner(SoftwareSerial* serial) : _serial(serial) {}

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

std::vector<Command> MHET_Live_Barcode_Scanner::getConfiguration()
{
    return _configuration;
}