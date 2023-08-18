#include <Menu.h>
#include <Arduino.h>

Menu::Menu(Logger& logger, Adafruit_SSD1306& oledDisplay, Adafruit_ADS1115& joystickMux, int8_t joystickButton) : _logger(logger), _oledDisplay(oledDisplay), _joystickMux(joystickMux), _joystickButton(joystickButton)
{
    _currentItemIndex = 0;
    _activeSubMenuIndex = 0;
    _displayAvailable = true;
    _joystickButtonLatch = false;
    _joystickDownLatch = false;
    _joystickLeftLatch = false;
    _joystickUpLatch = false;
    _joystickRightLatch = false;
    _menuRefreshPending = true;
}

void Menu::addItem(int8_t index, String label)
{
    _menuItems.emplace_back(MenuItem(index, label));
}

void Menu::drawMenuScanBarcode()
{

}

void Menu::drawMenuShowScannedAt()
{

}

void Menu::drawMenuRemoveProduct()
{

}

void Menu::drawMenuShowLog()
{
    _logger.rewindLog();
}

void Menu::loop()
{
    if (_activeSubMenuIndex > 0 && _activeSubMenuIndex < _menuItems.size())
    {
        switch (_activeSubMenuIndex)
        {
            case 1:
                drawMenuScanBarcode();
                break;
            case 2:
                drawMenuShowScannedAt();
                break;
            case 3:
                drawMenuRemoveProduct();
                break;
            case 4:
                drawMenuShowLog();
                break;
            default:
                _logger.log(Logger::LOG_COMPONENT_MENU, Logger::LOG_EVENT_WARN, "Invalid menu index");
        }
    }
    else
    {
        int adc0 = _joystickMux.readADC_SingleEnded(0);
        //int adc1 = _joystickMux.readADC_SingleEnded(1);
        long xAxisValue = map(adc0,0,32768,0,1000);
        //long yAxisValue = map(adc1,0,32768,0,1000);

        // Detect input down
        if (xAxisValue < 300 && _joystickDownLatch == false)
        {
            _joystickDownLatch = true;
            selectNextItem();
            _menuRefreshPending = true;;
        }
        else if (xAxisValue >= 300 && _joystickDownLatch == true)
        {
            _joystickDownLatch = false;
        }

        // Detect input up
        if (xAxisValue > 700 && _joystickUpLatch == false)
        {
            _joystickUpLatch = true;
            selectPreviousItem();
            _menuRefreshPending = true;;
        }
        else if (xAxisValue <= 700 && _joystickUpLatch == true)
        {
            _joystickUpLatch = false;
        }

        // Detect selection
        int switchStatus = digitalRead(_joystickButton);
        if (switchStatus == LOW && _joystickButtonLatch == false)
        {
            _joystickButtonLatch = true;
            executeSelectedItem();
            _menuRefreshPending = true;
        }
        else if (switchStatus == HIGH && _joystickButtonLatch == true)
        {
            _joystickButtonLatch = false;
        }

        if (_menuRefreshPending)
        {
            if (_displayAvailable)
            {
                _oledDisplay.clearDisplay();
                _menuRefreshPending = false;
            }

            for (u_int8_t i = 0; i < _menuItems.size(); ++i) 
            {
                String item;
                if (i == _currentItemIndex) 
                {
                    item += ">\t";
                } else 
                {
                    item += " \t";
                }
                item += _menuItems[i].getLabel();

                if (_displayAvailable)
                {
                    int nextLineCursor = 0;
                    if (i > 0)
                    {
                        nextLineCursor = i * 8 + 1;
                    }
                    _oledDisplay.setCursor(0, nextLineCursor);
                    _oledDisplay.println(item);
                    _oledDisplay.display();
                }
            }
        }
    }
}

void Menu::selectNextItem()
{
    _currentItemIndex = _currentItemIndex + 1;
    if (_currentItemIndex >= _menuItems.size())
    {
        _currentItemIndex = 0;
    }
}

void Menu::selectPreviousItem()
{
    _currentItemIndex = _currentItemIndex - 1;
    if (_currentItemIndex < 0)
    {
        _currentItemIndex = _menuItems.size() - 1;
    }
}

void Menu::executeSelectedItem()
{
    _activeSubMenuIndex = _currentItemIndex + 1;
}