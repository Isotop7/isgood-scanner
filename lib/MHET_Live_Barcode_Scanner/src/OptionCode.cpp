#include "OptionCode.h"

namespace OptionCodes
{
    static const std::map<OptionCode, String> optionCodeMap = {
        {OptionCode::Invalid, "Invalid"},
        {OptionCode::OneDimensionalCodes, "OneDimensionalCodes"},
        {OptionCode::TwoDimensionalCodes, "TwoDimensionalCodes"},
        {OptionCode::SystemScanMode, "SystemScanMode"},
        {OptionCode::ContinuousModeSleep, "ContinuousModeSleep"},
        {OptionCode::InductionModeSensingSensitivity, "InductionModeSensingSensitivity"},
        {OptionCode::HorizontalMirroring, "HorizontalMirroring"},
        {OptionCode::VerticalMirroring, "VerticalMirroring"},
        {OptionCode::InductionModeNightVision, "InductionModeNightVision"},
        {OptionCode::ResetOutput, "ResetOutput"},
        {OptionCode::TriggerModeAnswer, "TriggerModeAnswer"},
        {OptionCode::TriggerModeFillLight, "TriggerModeFillLight"},
        {OptionCode::SetupMode, "SetupMode"},
        {OptionCode::OutputLineBreaks, "OutputLineBreaks"},
        {OptionCode::OutputTab, "OutputTab"},
        {OptionCode::OutputAutoAddPrefix, "OutputAutoAddPrefix"},
        {OptionCode::OutputAutoAddSuffix, "OutputAutoAddSuffix"},
        {OptionCode::OutputAddCodeID, "OutputAddCodeID"},
        {OptionCode::ConvertOutputUpperLower, "ConvertOutputUpperLower"},
        {OptionCode::SameCodeIdentificationDelay, "SameCodeIdentificationDelay"},
        {OptionCode::SingleReadingTime, "SingleReadingTime"},
        {OptionCode::ReadingInterval, "ReadingInterval"},
        {OptionCode::ToneModeBootup, "ToneModeBootup"},
        {OptionCode::ToneModeDecode, "ToneModeDecode"},
        {OptionCode::ToneModeConfigure, "ToneModeConfigure"},
        {OptionCode::ToneVolumeBootup, "ToneVolumeBootup"},
        {OptionCode::ToneVolumeDecode, "ToneVolumeDecode"},
        {OptionCode::ToneVolumeConfiguration, "ToneVolumeConfiguration"},
        {OptionCode::ContinuousModeFillLight, "ContinuousModeFillLight"},
        {OptionCode::AimingLight, "AimingLight"},
        {OptionCode::IndicatorStartup, "IndicatorStartup"},
        {OptionCode::IndicatorDecoding, "IndicatorDecoding"},
        {OptionCode::IndicatorSuccessfulConfiguration, "IndicatorSuccessfulConfiguration"},
        {OptionCode::InductionModeFillLight, "InductionModeFillLight"},
        {OptionCode::EnableCodebar, "EnableCodebar"},
        {OptionCode::EnableCode128, "EnableCode128"},
        {OptionCode::EnableCode39, "EnableCode39"},
        {OptionCode::EnableInterleaved2Of5, "EnableInterleaved2Of5"},
        {OptionCode::EnableQR, "EnableQR"},
        {OptionCode::EnableUPCEANJAN, "EnableUPCEANJAN"},
        {OptionCode::EnableCode93, "EnableCode93"},
        {OptionCode::EnableIndustrial2Of5, "EnableIndustrial2Of5"},
        {OptionCode::EnableMatrix2Of5, "EnableMatrix2Of5"},
        {OptionCode::EnableCode11, "EnableCode11"},
        {OptionCode::EnableMSI, "EnableMSI"},
        {OptionCode::EnableGS1, "EnableGS1"},
        {OptionCode::EnableISBN, "EnableISBN"},
        {OptionCode::EnableISSN, "EnableISSN"},
        {OptionCode::EnableCODE32, "EnableCODE32"}
    };

    // Function to get the hex value for a given enum name
    uint16_t getHexValueForEnum(OptionCode optionCode) {
        auto it = optionCodeMap.find(optionCode);
        if (it != optionCodeMap.end()) {
            // Found the enum name in the map, return its hex value
            return static_cast<uint16_t>(optionCode);
        }
        // Enum name not found, return a default value (or handle as needed)
        return 0;
    }

    // Function to get the enum name for a given hex value
    OptionCode getEnumForHexValue(uint16_t hexValue) {
        for (const auto& pair : optionCodeMap) {
            if (static_cast<uint16_t>(pair.first) == hexValue) {
                // Found the hex value in the map, return the associated enum
                return pair.first;
            }
        }
        // Hex value not found, return a default enum value (or handle as needed)
        return OptionCode::Invalid;
    }

    // Function to get the map of enum values and names
    const std::map<OptionCode, String>& getOptionCodeMap() {
        return optionCodeMap;
    }
}