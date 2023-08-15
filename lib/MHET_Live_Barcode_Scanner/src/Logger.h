#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

class Logger
{
    public:
        static const String LOG_EVENT_INFO;
        static const String LOG_EVENT_WARN;
        static const String LOG_EVENT_ERROR;

        static const String LOG_COMPONENT_MAIN;
        static const String LOG_COMPONENT_SCANNER;
        static const String LOG_COMPONENT_CONFIG;
        static const String LOG_COMPONENT_OTA;
        static const String LOG_COMPONENT_WIFI;
        static const String LOG_COMPONENT_MQTT;

        void log(String component, String event, String message);
};

#endif // LOGGER_H