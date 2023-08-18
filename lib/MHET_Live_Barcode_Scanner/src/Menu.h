#ifndef MENU_H
#define MENU_H

#include <vector>
#include <Adafruit_SSD1306.h>
#include <Adafruit_ADS1X15.h>
#include <Logger.h>
#include <MenuItem.h>

class Menu
{
    private:
        const u_int16_t MENU_REDRAW_DELAY = 1000;
        
        std::vector<MenuItem> _menuItems;
        Adafruit_SSD1306& _oledDisplay;
        Adafruit_ADS1115& _joystickMux;
        Logger& _logger;
        bool _displayAvailable = false;
        int8_t _currentItemIndex;
        int8_t _activeSubMenuIndex;
        int8_t _joystickButton;
        bool _joystickButtonLatch;
        bool _joystickDownLatch;
        bool _joystickLeftLatch;
        bool _joystickUpLatch;
        bool _joystickRightLatch;
        bool _menuRefreshPending;
        void drawMenuScanBarcode();
        void drawMenuShowScannedAt();
        void drawMenuRemoveProduct();
        void drawMenuShowLog();

    public:
        Menu(Logger& logger, Adafruit_SSD1306& oledDisplay, Adafruit_ADS1115& joystickMux, int8_t joystickButton);
        void addItem(int8_t index, String label);
        void loop();
        void selectNextItem();
        void selectPreviousItem();
        void executeSelectedItem();
};

#endif // MENU_H