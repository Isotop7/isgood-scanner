#ifndef MENUITEM_H
#define MENUITEM_H

#include <Arduino.h>

class MenuItem
{
    private:
         int8_t _index;
         String _label;
    public:
        MenuItem(int8_t index, String label);
        int8_t getIndex();
        String getLabel();
};

#endif // MENUITEM_H