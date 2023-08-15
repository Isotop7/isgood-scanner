#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>

class Logger
{
    public:
        Logger(Adafruit_SSD1306& oledDisplay);

        static const String LOG_EVENT_INFO;
        static const String LOG_EVENT_WARN;
        static const String LOG_EVENT_ERROR;

        static const String LOG_COMPONENT_MAIN;
        static const String LOG_COMPONENT_SCANNER;
        static const String LOG_COMPONENT_CONFIG;
        static const String LOG_COMPONENT_OTA;
        static const String LOG_COMPONENT_WIFI;
        static const String LOG_COMPONENT_MQTT;
        static const String LOG_COMPONENT_DISPLAY;

        void log(String component, String event, String message);
    private:
        Adafruit_SSD1306& _oledDisplay;
        bool _displayAvailable = false;
};

#endif // LOGGER_H