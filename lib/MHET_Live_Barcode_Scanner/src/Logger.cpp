#include "Logger.h"

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <LogMessage.h>

const String Logger::LOG_EVENT_INFO = "INFO";
const String Logger::LOG_EVENT_WARN = "WARNING";
const String Logger::LOG_EVENT_ERROR = "ERROR";

const String Logger::LOG_COMPONENT_MAIN = "ESP8266";
const String Logger::LOG_COMPONENT_SCANNER = "SCANNER";
const String Logger::LOG_COMPONENT_CONFIG = "CONFIGURATION";
const String Logger::LOG_COMPONENT_OTA = "OTA";
const String Logger::LOG_COMPONENT_WIFI = "WIFI";
const String Logger::LOG_COMPONENT_MQTT = "MQTT";
const String Logger::LOG_COMPONENT_DISPLAY = "DISPLAY";
const String Logger::LOG_COMPONENT_JOYSTICK = "JOYSTICK";

Logger::Logger(Adafruit_SSD1306 &oledDisplay) : _oledDisplay(oledDisplay), _lastLogMessage(LogMessage("", "", ""))
{
  _displayAvailable = true;
}

void Logger::log(String component, String event, String message)
{
  _lastLogMessage = LogMessage(component, event, message);
  Serial.println("+ <" + component + "> | [" + event + "] : " + message);

  if (_displayAvailable)
  {
    _oledDisplay.clearDisplay();
    _oledDisplay.setCursor(0, 0);
    _oledDisplay.println("Component: " + component);
    _oledDisplay.setCursor(0, 9);
    _oledDisplay.println("Event: " + event);
    _oledDisplay.setCursor(0, 17);
    _oledDisplay.println("Message: " + message);
    _oledDisplay.display();
  }
}

void Logger::rewindLog()
{
  log(_lastLogMessage.getComponent(), _lastLogMessage.getEvent(), _lastLogMessage.getMessage());
}

LogMessage Logger::getLastLogMessage()
{
  return _lastLogMessage;
}