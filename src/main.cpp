#include <Arduino.h>
#include <SoftwareSerial.h>
#include <MHET_Live_Barcode_Scanner.h>
#include <CommandResponse.h>

const unsigned long SERIAL_TIMEOUT = 2000;
const unsigned long SERIAL_BUFFER_TIMEOUT = 50;
const bool RESET_SCANNER = false;
const bool ENABLE_DEBUG = false;

const String LOG_EVENT_INFO = "INFO";
const String LOG_EVENT_WARN = "WARNING";
const String LOG_EVENT_ERROR = "ERROR";

const String LOG_COMPONENT_MAIN = "ESP8266";
const String LOG_COMPONENT_SCANNER = "SCANNER";
const String LOG_COMPONENT_CONFIG = "CONFIGURATION";

SoftwareSerial scannerSerial(D1, D2);
MHET_Live_Barcode_Scanner scanner(&scannerSerial);

void writeLog(String component, String event, String message)
{
  Serial.println("+ <" + component + "> | [" + event + "] : " + message);
}

void applyCommand(char command[])
{
  writeLog(LOG_COMPONENT_CONFIG, LOG_EVENT_INFO, "Sending command -> " + String(command));

  scannerSerial.write(command);
  delay(SERIAL_BUFFER_TIMEOUT);
  if (scannerSerial.available())
  {
    char response = scannerSerial.read();
    writeLog(LOG_COMPONENT_CONFIG, LOG_EVENT_INFO, "Response:\t" + String(response));

    if (response == CommandResponse::ACK)
    {
      writeLog(LOG_COMPONENT_CONFIG, LOG_EVENT_INFO, "Command successful");
    }
    else if (response == CommandResponse::ENQ)
    {
      writeLog(LOG_COMPONENT_CONFIG, LOG_EVENT_WARN, "Invalid command");
    }
    else
    {
      writeLog(LOG_COMPONENT_CONFIG, LOG_EVENT_WARN, "Unknown response -> " + String(response));
    }
  }
  delay(SERIAL_BUFFER_TIMEOUT);
}

void sendCommand(char command[])
{
  writeLog(LOG_COMPONENT_CONFIG, LOG_EVENT_INFO, "Sending command -> " + String(command));
  scannerSerial.write(command);
}

void resetAllSettings()
{
  applyCommand("~M00910001.\n");
  applyCommand("~MA5F01B2C.\n");
  applyCommand("~MA5F0D201.\n");
  applyCommand("~M00510000.\n");
  applyCommand("~MA5F0506A.\n");
  applyCommand("~M00910000.\n");
}

void setupScanner()
{
  applyCommand("~M00910001.\n");
  
  applyCommand("~M00510000.\n");
  applyCommand("~M00210000.\n");
  applyCommand("~M00FA0002.\n");
  applyCommand("~M00FB0003.\n");
  applyCommand("~M00FC0002.\n");

  applyCommand("~MA5F0506A.\n");
  applyCommand("~M00910000.\n");
}

void enableOutput()
{
  applyCommand("~M00910001.");
  applyCommand("~M00510000.");
  applyCommand("~MA5F0506A.");
  applyCommand("~M00910000.");
}

void showConfig()
{
  String config;
  // Init config mode
  applyCommand("~M00910001.\n");
  // Query config
  sendCommand("~QFA50.\n");
  // Wait for data to arrive in buffer
  delay(SERIAL_BUFFER_TIMEOUT);
  
  // Read data from buffer
  while (scannerSerial.available())
  {
    char key = scannerSerial.read();
    // Add key to config
    config += key;

    // If we cant find data, wait for the serial timeout to be sure no more data has arrived while emptying buffer
    if (scannerSerial.available() == 0)
    {
      delay(SERIAL_BUFFER_TIMEOUT);
    }
  }

  int configSize = config.length();
  writeLog(LOG_COMPONENT_CONFIG, LOG_EVENT_INFO, "Size of config data: " + configSize);

  char rawConfig[configSize];
  config.toCharArray(rawConfig, configSize, 1);
  scanner.readConfig(rawConfig, configSize);
  std::vector<Command> configuration = scanner.getConfiguration();

  int commandElements = 0;
  for (const Command& obj : configuration) {
      String message = "OptionCode -> ";
      message += obj.getOptionCode();
      message += "; Value -> ";
      message += String(obj.getValue(), HEX);
      writeLog(LOG_COMPONENT_CONFIG, LOG_EVENT_INFO, message);
      commandElements++;
  }
  writeLog(LOG_COMPONENT_CONFIG, LOG_EVENT_INFO, "Number of configuration commands: " + commandElements);

  //Serial.println(config);
  applyCommand("~M00910000.\n");
}

void readScannerSerial()
{
  String barcode;
  char receivedChar;

  while (scannerSerial.available())
  {
    receivedChar = scannerSerial.read();
    if (receivedChar == CommandResponse::EOL) {
      writeLog(LOG_COMPONENT_SCANNER, LOG_EVENT_INFO, "New barcode scanned -> " + barcode);
      break; // Exit the loop if line feed character is received
    }
    barcode += receivedChar;

    // If we cant find data, wait for the serial timeout to be sure no more data has arrived while emptying buffer
    if (scannerSerial.available() == 0)
    {
      delay(SERIAL_BUFFER_TIMEOUT);
    }
  }
}

void setup() {
  Serial.begin(9600);
  delay(1000);
  writeLog(LOG_COMPONENT_MAIN, LOG_EVENT_INFO, "Setup external serial monitor");
  scannerSerial.begin(9600);

  if (RESET_SCANNER)
  {
    writeLog(LOG_COMPONENT_MAIN, LOG_EVENT_WARN, "Reset all settings");
    resetAllSettings();
    writeLog(LOG_COMPONENT_MAIN, LOG_EVENT_INFO, "Set up scanner");
    setupScanner();
  }

  writeLog(LOG_COMPONENT_MAIN, LOG_EVENT_INFO, "Show config");
  showConfig();
  writeLog(LOG_COMPONENT_MAIN, LOG_EVENT_INFO, "Leave setup()");
}

void loop() {
  readScannerSerial();
}