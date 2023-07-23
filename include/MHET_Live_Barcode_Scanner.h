#ifndef MHET_LIVE_BARCODE_SCANNER_H
#define MHET_LIVE_BARCODE_SCANNER_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <map>
#include <string>

class MHET_Live_Barcode_Scanner {
public:
  MHET_Live_Barcode_Scanner(uint32_t _rx, uint32_t _tx);              
  enum ScanMode {                           
    TRIGGER,
    CONTINOUS
  };
  enum Code {                               
    FactoryReset,
    SaveUserDefaults,
    RestoreUserDefaults
  };                                
  std::map<Code, std::string> CodeMap = {     
    {FactoryReset,        "~MA5F01B2C."},
    {SaveUserDefaults,    "~MA5F0506A."},
    {RestoreUserDefaults, "~MA5F08F37."}
  };

  void begin(Stream &serial);
  void setMode(ScanMode mode);
  String dumpConfig();
  String readBarcode();

private:
    uint32_t _tx;
    uint32_t _rx;
    SoftwareSerial _sserial(uint32_t _rx, uint32_t _tx);
};

#endif // MHET_LIVE_BARCODE_SCANNER_H
