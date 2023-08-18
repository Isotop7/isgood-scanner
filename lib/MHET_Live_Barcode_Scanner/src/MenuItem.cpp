#include <MenuItem.h>
#include <Arduino.h>

MenuItem::MenuItem(int8_t index, String label) : _index(index), _label(label) {}

int8_t MenuItem::getIndex()
{
    return _index;
}

String MenuItem::getLabel()
{
    return _label;
}