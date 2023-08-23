#ifndef LOGMESSAGE_H
#define LOGMESSAGE_H

#include <Arduino.h>

class LogMessage
{
public:
    LogMessage(String component, String event, String message) : _component(component), _event(event), _message(message){};

    const String getComponent() const
    {
        return _component;
    }
    const String getEvent() const
    {
        return _event;
    }
    const String getMessage() const
    {
        return _message;
    }

private:
    String _component;
    String _event;
    String _message;
};

#endif // LOGMESSAGE_H