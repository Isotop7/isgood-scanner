#ifndef MHET_LIVE_BARCODE_SCANNER_H
#define MHET_LIVE_BARCODE_SCANNER_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <map>
#include <string>

class MHET_Live_Barcode_Scanner {
public:
  MHET_Live_Barcode_Scanner();              // Constructor
  enum ScanMode {                           // Enum for mode of scanning
    TRIGGER,
    CONTINOUS
  };
  enum Code {                               // String values for codes

  };                                
  std::map<Code, std::string> CodeMap {     // Code map

  };

  void begin(Stream &serial);               // Initialize the barcode scanner
  void setMode(ScanMode mode);              // Sets operation mode
  String dumpConfig();                      // Retrieves config and dumps it
  String readBarcode();                     // Read and return the scanned barcode

private:
    uint32_t _tx;
    uint32_t _rx;
    SoftwareSerial _sserial(uint32_t _rx, uint32_t _tx);
};

#endif // MHET_LIVE_BARCODE_SCANNER_H
