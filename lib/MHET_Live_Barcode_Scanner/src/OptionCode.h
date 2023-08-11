#ifndef OPTIONCODE_H
#define OPTIONCODE_H

#include <Arduino.h>
#include <map>

namespace OptionCodes
{
  enum class OptionCode {
    Invalid                                 = 0x0000,
    OneDimensionalCodes                     = 0x0001,
    TwoDimensionalCodes                     = 0x0002,
    SystemScanMode                          = 0x0021,
    ContinuousModeSleep                     = 0x0022,
    InductionModeSensingSensitivity         = 0x0023,
    HorizontalMirroring                     = 0x0024,
    VerticalMirroring                       = 0x0025,
    InductionModeNightVision                = 0x0026,
    ResetOutput                             = 0x0051,
    TriggerModeAnswer                       = 0x0073,
    TriggerModeFillLight                    = 0x0086,
    SetupMode                               = 0x0091,
    OutputLineBreaks                        = 0x0092,
    OutputTab                               = 0x0093,
    OutputAutoAddPrefix                     = 0x0094,
    OutputAutoAddSuffix                     = 0x0095,
    OutputAddCodeID                         = 0x009B,
    ConvertOutputUpperLower                 = 0x009C,
    SameCodeIdentificationDelay             = 0x00B0,
    SingleReadingTime                       = 0x00B1,
    ReadingInterval                         = 0x00B2,
    ToneModeBootup                          = 0x00EA,
    ToneModeDecode                          = 0x00EB,
    ToneModeConfigure                       = 0x00EC,
    ToneVolumeBootup                        = 0x00FA,
    ToneVolumeDecode                        = 0x00FB,
    ToneVolumeConfiguration                 = 0x00FC,
    ContinuousModeFillLight                 = 0x0103,
    AimingLight                             = 0x0105,
    IndicatorStartup                        = 0x010A,
    IndicatorDecoding                       = 0x010B,
    IndicatorSuccessfulConfiguration        = 0x010C,
    InductionModeFillLight                  = 0x0126,
    EnableCodebar                           = 0x0145,
    EnableCode128                           = 0x0150,
    EnableCode39                            = 0x0160,
    EnableInterleaved2Of5                   = 0x0185,
    EnableQR                                = 0x01B0,
    EnableUPCEANJAN                         = 0x01BA,
    EnableCode93                            = 0x01C0,
    EnableIndustrial2Of5                    = 0x01E5,
    EnableMatrix2Of5                        = 0x0200,
    EnableCode11                            = 0x1000,
    EnableMSI                               = 0x1100,
    EnableGS1                               = 0x1200,
    EnableISBN                              = 0x1300,
    EnableISSN                              = 0x1326,
    EnableCODE32                            = 0x1353
  };

  uint16_t getHexValueForEnum(OptionCode code);
  OptionCode getEnumForHexValue(uint16_t hexValue);
  const std::map<OptionCode, String>& getOptionCodeMap();
}

#endif // OPTIONCODE_H