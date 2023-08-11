#ifndef MHET_LIVE_BARCODE_SCANNER_H
#define MHET_LIVE_BARCODE_SCANNER_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <vector>
#include "OptionCode.h"
#include "Command.h"

class MHET_Live_Barcode_Scanner {
public:
  MHET_Live_Barcode_Scanner(SoftwareSerial* serial);
  
  bool configureOption(uint16_t optionCode, int value);
  void readConfig(const char* rawConfig, size_t length);
  std::vector<Command> getConfiguration();

private:
  SoftwareSerial* _serial;
  std::vector<Command> _configuration;
  void generateCommand();
  bool sendCommand();
};

#endif // MHET_LIVE_BARCODE_SCANNER_H
