#ifndef MHET_LIVE_BARCODE_SCANNER_H
#define MHET_LIVE_BARCODE_SCANNER_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <vector>
#include "OptionCode.h"
#include "Command.h"
#include "Logger.h"

class MHET_Live_Barcode_Scanner
{
public:
  MHET_Live_Barcode_Scanner(SoftwareSerial *serial, unsigned long timeout, Logger &loggerRef);
  void resetSettings();
  void enableOutput();

  Command::Response configureOption(int optionCode, int value);
  void setOption(int optionCode, int value);
  void getOption(int optionCode);

  Command::Response queryConfiguration();
  void readConfig(const char *rawConfig, size_t length);
  std::vector<Command> getConfigurationInstance() const;
  String getConfigurationAsString() const;
  String getConfigurationAsHTML() const;

  String getNextBarcode() const;

private:
  SoftwareSerial *_serial;
  unsigned long _timeout;
  Logger &_logger;
  std::vector<Command> _configuration;
  void generateCommand();
  bool sendCommand();
};

#endif // MHET_LIVE_BARCODE_SCANNER_H
