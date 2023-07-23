#ifndef MHET_LIVE_BARCODE_SCANNER_H
#define MHET_LIVE_BARCODE_SCANNER_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <map>
#include <string>

class MHET_Live_Barcode_Scanner {
public:
  MHET_Live_Barcode_Scanner();              // Constructor
  enum ScanMode;                            // Enum for mode of scanning
  enum Code {                               // String values for codes

  };                                
  std::map<Code, std::string> CodeMap {     // Code map

  };

  void begin(Stream &serial);               // Initialize the barcode scanner
  void setMode(ScanMode mode);              // Sets operation mode
  String dumpConfig();                      // Retrieves config and dumps it
  String readBarcode();                     // Read and return the scanned barcode

private:
    int _tx;
    int _rx
    SoftwareSerial _sserial(_rx, _tx)
};

#endif // MHET_LIVE_BARCODE_SCANNER_H
