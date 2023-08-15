#include "Logger.h"

#include <Arduino.h>


const String Logger::LOG_EVENT_INFO = "INFO";
const String Logger::LOG_EVENT_WARN = "WARNING";
const String Logger::LOG_EVENT_ERROR = "ERROR";

const String Logger::LOG_COMPONENT_MAIN = "ESP8266";
const String Logger::LOG_COMPONENT_SCANNER = "SCANNER";
const String Logger::LOG_COMPONENT_CONFIG = "CONFIGURATION";
const String Logger::LOG_COMPONENT_OTA = "OTA";
const String Logger::LOG_COMPONENT_WIFI = "WIFI";
const String Logger::LOG_COMPONENT_MQTT = "MQTT";

void Logger::log(String component, String event, String message)
{
  Serial.println("+ <" + component + "> | [" + event + "] : " + message);
}