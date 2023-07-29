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
    RestoreUserDefaults,
    DeleteUserDefaults,
    SetupModeOn,
    SetupModeOff,
    ReadModeTrigger,
    ReadModeContinous,
    ReadModeInduction,
    ReadModeCommand,
    InductionModeNightVisionOff,
    InductionModeNightVisionOn,
    ContinousModeSleepOff,
    ContinousModeSleepOn,
    MirrorHorizontalOff,
    MirrorHorizontalOn,
    MirrorVerticalOff,
    MirrorVerticalOn,
    AutomaticModeSameReadDelayOff,
    AutomaticModeSameReadDelay100ms,
    AutomaticModeSameReadDelay1s,
    AutomaticModeSameReadDelay10s,
    AutomaticModeSingleReadingTimeDelayOff,
    AutomaticModeSingleReadingTimeDelay5s
  };                                
  std::map<Code, std::string> CodeMap = {     
    {FactoryReset,                                "~MA5F01B2C."},
    {SaveUserDefaults,                            "~MA5F0506A."},
    {RestoreUserDefaults,                         "~MA5F08F37."},
    {DeleteUserDefaults,                          "~MA5F0D201."},
    {SetupModeOn,                                 "~M00910001."},
    {SetupModeOff,                                "~M00910000."},
    {ReadModeTrigger,                             "~M00210000."},
    {ReadModeContinous,                           "~M00210001."},
    {ReadModeCommand,                             "~M00210003."},
    {ReadModeInduction,                           "~M00210002."},
    {InductionModeNightVisionOff,                 "~M00260000."},
    {InductionModeNightVisionOn,                  "~M00260001."},
    {ContinousModeSleepOff,                       "~M00220000."},
    {ContinousModeSleepOn,                        "~M00220001."},
    {MirrorHorizontalOff,                         "~M00240000."},
    {MirrorHorizontalOn,                          "~M00240001."},
    {MirrorVerticalOff,                           "~M00250000."},
    {MirrorVerticalOn,                            "~M00250001."},
    {AutomaticModeSameReadDelayOff,               "~M00B00000."},
    {AutomaticModeSameReadDelay100ms,             "~M00B00001."}, // 1    x 100ms as hex
    {AutomaticModeSameReadDelay1s,                "~M00B0000A."}, // 10   x 100ms as hex
    {AutomaticModeSameReadDelay10s,               "~M00B00064."}, // 100  x 100ms as hex
    {AutomaticModeSingleReadingTimeDelayOff,      "~M00B10000."}, // 0     x 100ms as hex
    {AutomaticModeSingleReadingTimeDelay5s,       "~M00B10032."}, // 50    x 100ms as hex
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
