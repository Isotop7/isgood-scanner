#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_ADS1X15.h>
#include <CountDown.h>

#include <MHET_Live_Barcode_Scanner.h>
#include <Command.h>
#include <Product.h>
#include <Logger.h>

#include "Settings.h"

const bool RESET_SCANNER = false;
const bool ENABLE_DEBUG = false;
const String FIRMWARE_VERSION = "0.3.4";

Adafruit_SSD1306 oledDisplay(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET_PIN);
Adafruit_ADS1115 analogMux;
Logger logger(oledDisplay);
SoftwareSerial scannerSerial(SCANNER_RX_PIN, SCANNER_TX_PIN);
MHET_Live_Barcode_Scanner scanner(&scannerSerial, SCANNER_SERIAL_BUFFER_TIMEOUT, logger);
WiFiClient espClient;
PubSubClient mqttClient(espClient);
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

// MENU
bool menuRefreshPending = true;
bool menuItemRefreshPending;
int8_t menuItemsLimit = 4;
String menuItems[4] = {"Scan Barcode", "Show ScannedAt", "Remove product", "Show log"};
enum menuIndex
{
    MAIN_MENU = 0,
    SCAN_BARCODE = 1,
    SHOW_SCANNEDAT = 2,
    REMOVE_PRODUCT = 3,
    SHOW_LOG = 4,
    SET_TIMESTAMP = 5
};
int8_t selectedMenuIndex = 0;
int8_t activeMenuIndex = 0;
// DISPLAY
bool displayAvailable = true;
// JOYSTICK
bool joystickButtonLatch = false;
bool joystickDownLatch = false;
bool joystickLeftLatch = false;
bool joystickUpLatch = false;
bool joystickRightLatch = false;
// TIMESTAMP
int16_t timestamp[3] = {1, 1, 2023};
const u_int8_t daysOfMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
int8_t selectedTimestampDigit = 0;
bool timestampRefreshPending = true;
// SCANNER
Product product;
CountDown bestBeforeTimeout(CountDown::Resolution::SECONDS);

void setupScanner()
{
    scanner.configureOption(OptionCodes::getHexValueForEnum(OptionCodes::OptionCode::SetupMode), OptionCodes::Values::SetupMode::ACTIVE);
    scanner.configureOption(OptionCodes::getHexValueForEnum(OptionCodes::OptionCode::SystemScanMode), OptionCodes::Values::ScanMode::TRIGGER);
    scanner.configureOption(OptionCodes::getHexValueForEnum(OptionCodes::OptionCode::ToneVolumeBootup), 2);
    scanner.configureOption(OptionCodes::getHexValueForEnum(OptionCodes::OptionCode::ToneVolumeDecode), 3);
    scanner.configureOption(OptionCodes::getHexValueForEnum(OptionCodes::OptionCode::ToneVolumeConfiguration), 2);
    scanner.configureOption(OptionCodes::getHexValueForEnum(OptionCodes::OptionCode::DeviceSettings), 0x506A);
    scanner.configureOption(OptionCodes::getHexValueForEnum(OptionCodes::OptionCode::SetupMode), OptionCodes::Values::SetupMode::INACTIVE);
}

void showConfig()
{
    Command::Response response = scanner.queryConfiguration();

    if (response == Command::Response::ACK)
    {
        std::vector<Command> configuration = scanner.getConfigurationInstance();

        int commandElements = 0;
        for (const Command &obj : configuration)
        {
            String message = "OptionCode -> ";
            message += obj.getOptionCode();
            message += "; Value -> ";
            message += String(obj.getValue(), HEX);
            logger.log(Logger::LOG_COMPONENT_CONFIG, Logger::LOG_EVENT_INFO, message);
            commandElements++;
        }
        logger.log(Logger::LOG_COMPONENT_CONFIG, Logger::LOG_EVENT_INFO, "Number of configuration commands: " + commandElements);
    }
}

void handleRoot()
{
    String response = F("<!DOCTYPE html>\n"
                        "<html lang='en'>\n"
                        "<head>\n"
                        "<title>isgood-scanner</title>\n"
                        "</head>\n"
                        "<link rel=\"stylesheet\" href=\"https://unpkg.com/sakura.css/css/sakura.css\" type=\"text/css\">\n"
                        "<body>\n"
                        "<h1>isgood-scanner</h1>\r\n");
    response += "<h2>firmware version: " + FIRMWARE_VERSION + "</h2>\r\n";
    response += "<h3>Current configuration:</h3>";
    response += scanner.getConfigurationAsHTML() + "\r\n";
    response += "</body></html>";
    httpServer.send(200, "text/html", response);
}

void drawMenuScanBarcode()
{
    if (menuItemRefreshPending)
    {
        oledDisplay.clearDisplay();
        oledDisplay.setCursor(0, 0);
        oledDisplay.println("Waiting for new");
        oledDisplay.println("barcode ...");
        oledDisplay.display();

        menuItemRefreshPending = false;
    }

    product = Product(scanner.getNextBarcode());
    if (product.isValid())
    {
        logger.log(Logger::LOG_COMPONENT_MQTT, Logger::LOG_EVENT_INFO, "Publish new barcode " + product.getBarcode());
        mqttClient.publish(ISGOOD_TOPIC_BARCODE, product.getBarcodeJSON().c_str());
        bestBeforeTimeout.start(ISGOOD_CONFIG_BESTBEFORETIMEOUT);
        activeMenuIndex = menuIndex::SET_TIMESTAMP;
        menuRefreshPending = true;
    }
}

void drawMenuShowScannedAt()
{
    if (menuItemRefreshPending)
    {
        oledDisplay.clearDisplay();
        oledDisplay.setCursor(0, 0);
        oledDisplay.println("Waiting for new");
        oledDisplay.println("barcode ...");
        oledDisplay.display();

        menuItemRefreshPending = false;
    }

    product = Product(scanner.getNextBarcode());
    if (product.isValid())
    {
        logger.log(Logger::LOG_COMPONENT_MQTT, Logger::LOG_EVENT_INFO, "Publish new barcode to get scannedAt " + product.getBarcode());
        mqttClient.publish(ISGOOD_TOPIC_SCANNEDAT_GET, product.getBarcodeJSON().c_str());
        menuRefreshPending = true;
    }
}

void drawMenuRemoveProduct()
{
    if (menuItemRefreshPending)
    {
        logger.log(Logger::LOG_COMPONENT_MAIN, Logger::LOG_EVENT_ERROR, "Not implemented!");
        menuItemRefreshPending = false;
    }
}

void drawMenuShowLog()
{
    if (menuItemRefreshPending)
    {
        logger.rewindLog();
        menuItemRefreshPending = false;
    }
}

void decreaseTimestampDigit()
{
    timestamp[selectedTimestampDigit] = timestamp[selectedTimestampDigit] - 1;

    // Check for day overflow
    if (timestamp[0] < 1)
    {
        timestamp[1] = timestamp[1] - 1;
        timestamp[0] = daysOfMonth[timestamp[1]];
    }

    // Check if month does overflow
    if (timestamp[1] < 1)
    {
        // Reset to january
        timestamp[1] = 12;
        // Decrement year
        timestamp[2] = timestamp[2] - 1;
    }
}

void increaseTimestampDigit()
{
    timestamp[selectedTimestampDigit] = timestamp[selectedTimestampDigit] + 1;

    // Check for day overflow
    if (timestamp[0] > daysOfMonth[timestamp[1]])
    {
        timestamp[0] = 1;
        timestamp[1] = timestamp[1] + 1;
    }

    // Check if month does overflow
    if (timestamp[1] > 12)
    {
        // Reset to january
        timestamp[1] = 1;
        // Increment year
        timestamp[2] = timestamp[2] + 1;
    }
}

void moveTimestampDigit()
{
    selectedTimestampDigit = selectedTimestampDigit + 1;

    // Check for overflow
    if (selectedTimestampDigit > 2)
    {
        selectedTimestampDigit = 0;
    }
}

void publishTimestamp()
{
    product.setBestBefore(timestamp);
    logger.log(Logger::LOG_COMPONENT_MQTT, Logger::LOG_EVENT_INFO, String("Publish new bestBeforeDate for product " + product.getBarcode() + " with bestBefore '" + product.getBestBefore()) + "'");
    mqttClient.publish(ISGOOD_TOPIC_BESTBEFORE_SET, product.getBestBeforeJSON().c_str());
}

void drawMenuSetTimestamp()
{
    if (bestBeforeTimeout.isStopped())
    {
        activeMenuIndex = menuIndex::MAIN_MENU;
        selectedMenuIndex = 0;
        menuRefreshPending = true;
        menuItemRefreshPending = true;
        return;
    }

    // Read joystick
    int adc0 = analogMux.readADC_SingleEnded(0);
    int adc1 = analogMux.readADC_SingleEnded(1);
    long xAxisValue = map(adc0, 0, 32768, 0, 1000);
    long yAxisValue = map(adc1, 0, 32768, 0, 1000);

    // Detect input down -> decrease digit
    if (xAxisValue < 300 && joystickDownLatch == false)
    {
        joystickDownLatch = true;
        decreaseTimestampDigit();
    }
    else if (xAxisValue >= 300 && joystickDownLatch == true)
    {
        joystickDownLatch = false;
    }

    // Detect input up -> increase digit
    if (xAxisValue > 700 && joystickUpLatch == false)
    {
        joystickUpLatch = true;
        increaseTimestampDigit();
    }
    else if (xAxisValue <= 700 && joystickUpLatch == true)
    {
        joystickUpLatch = false;
    }

    // Detect input right -> next digit
    if (yAxisValue > 700 && joystickRightLatch == false)
    {
        joystickRightLatch = true;
        moveTimestampDigit();
    }
    else if (yAxisValue <= 700 && joystickRightLatch == true)
    {
        joystickRightLatch = false;
    }

    // Detect selection -> save timestamp
    int switchStatus = digitalRead(JOYSTICK_SWITCH_PIN);
    if (switchStatus == LOW && joystickButtonLatch == false)
    {
        joystickButtonLatch = true;
        publishTimestamp();
        selectedMenuIndex = 0;
        activeMenuIndex = menuIndex::MAIN_MENU;
        menuRefreshPending = true;
        return;
    }
    else if (switchStatus == HIGH && joystickButtonLatch == true)
    {
        joystickButtonLatch = false;
    }

    if (displayAvailable)
    {
        oledDisplay.clearDisplay();
        menuRefreshPending = false;
    }

    oledDisplay.setCursor(40, 0);
    oledDisplay.print(timestamp[0]);
    oledDisplay.print(".");
    oledDisplay.print(timestamp[1]);
    oledDisplay.print(".");
    oledDisplay.print(timestamp[2]);

    oledDisplay.setCursor((36 + selectedTimestampDigit * 12), 10);
    if (selectedTimestampDigit == 2)
    {
        oledDisplay.print("~~~~~");
    }
    else
    {
        oledDisplay.print("~~");
    }

    oledDisplay.setCursor(22, 18);
    oledDisplay.print(bestBeforeTimeout.remaining());
    oledDisplay.println(" seconds left");
    oledDisplay.display();
}

void drawShowScannedAt(String date)
{
    oledDisplay.clearDisplay();
    oledDisplay.setCursor(32, 8);
    oledDisplay.print("Scanned at: ");
    oledDisplay.setCursor(33, 22);
    oledDisplay.println(date);
    oledDisplay.display();
}

void selectNextItem()
{
    selectedMenuIndex = selectedMenuIndex + 1;
    if (selectedMenuIndex >= menuItemsLimit)
    {
        selectedMenuIndex = 0;
    }
}

void selectPreviousItem()
{
    selectedMenuIndex = selectedMenuIndex - 1;
    if (selectedMenuIndex < 0)
    {
        selectedMenuIndex = menuItemsLimit - 1;
    }
}

void executeSelectedItem()
{
    activeMenuIndex = selectedMenuIndex + 1;
    menuItemRefreshPending = true;
}

void mainLoop()
{
    if (activeMenuIndex == menuIndex::MAIN_MENU)
    {
        // Reset latches
        menuItemRefreshPending = true;
        joystickLeftLatch = false;
        joystickRightLatch = false;
        // Read joystick
        int adc0 = analogMux.readADC_SingleEnded(0);
        long xAxisValue = map(adc0, 0, 32768, 0, 1000);

        // Detect input down
        if (xAxisValue < 300 && joystickDownLatch == false)
        {
            joystickDownLatch = true;
            selectNextItem();
            menuRefreshPending = true;
            ;
        }
        else if (xAxisValue >= 300 && joystickDownLatch == true)
        {
            joystickDownLatch = false;
        }

        // Detect input up
        if (xAxisValue > 700 && joystickUpLatch == false)
        {
            joystickUpLatch = true;
            selectPreviousItem();
            menuRefreshPending = true;
            ;
        }
        else if (xAxisValue <= 700 && joystickUpLatch == true)
        {
            joystickUpLatch = false;
        }

        // Detect selection
        int switchStatus = digitalRead(JOYSTICK_SWITCH_PIN);
        if (switchStatus == LOW && joystickButtonLatch == false)
        {
            joystickButtonLatch = true;
            executeSelectedItem();
            menuRefreshPending = true;
        }
        else if (switchStatus == HIGH && joystickButtonLatch == true)
        {
            joystickButtonLatch = false;
        }

        if (menuRefreshPending)
        {
            if (displayAvailable)
            {
                oledDisplay.clearDisplay();
                menuRefreshPending = false;
            }

            for (u_int8_t i = 0; i < menuItemsLimit; ++i)
            {
                String item;
                if (i == selectedMenuIndex)
                {
                    item += ">\t";
                }
                else
                {
                    item += " \t";
                }
                item += menuItems[i];

                if (displayAvailable)
                {
                    int nextLineCursor = 0;
                    if (i > 0)
                    {
                        nextLineCursor = i * 8 + 1;
                    }
                    oledDisplay.setCursor(0, nextLineCursor);
                    oledDisplay.println(item);
                    oledDisplay.display();
                }
            }
        }
    }
    else
    {
        int adc1 = analogMux.readADC_SingleEnded(1);
        long yAxisValue = map(adc1, 0, 32768, 0, 1000);

        if (yAxisValue < 300 && joystickLeftLatch == false && activeMenuIndex != menuIndex::SET_TIMESTAMP)
        {
            joystickLeftLatch = true;
            activeMenuIndex = 0;
            menuRefreshPending = true;
        }
        else
        {
            switch (activeMenuIndex)
            {
            case menuIndex::SCAN_BARCODE:
                drawMenuScanBarcode();
                break;
            case menuIndex::SHOW_SCANNEDAT:
                drawMenuShowScannedAt();
                break;
            case menuIndex::REMOVE_PRODUCT:
                drawMenuRemoveProduct();
                break;
            case menuIndex::SHOW_LOG:
                drawMenuShowLog();
                break;
            case menuIndex::SET_TIMESTAMP:
                drawMenuSetTimestamp();
                break;
            default:
                logger.log(Logger::LOG_COMPONENT_MAIN, Logger::LOG_EVENT_WARN, "Invalid menu index");
            }
        }
    }
}

String getDateFromJSON(String json)
{
    String dateObject = json.substring(json.indexOf(','));
    dateObject = dateObject.substring(dateObject.indexOf(':'));
    dateObject = dateObject.substring(2, dateObject.indexOf('T'));

    String remainder;
    String year = dateObject.substring(0, dateObject.indexOf('-'));
    remainder = dateObject.substring(dateObject.indexOf('-') + 1);
    String month = remainder.substring(0, remainder.indexOf('-'));
    remainder = remainder.substring(remainder.indexOf('-') + 1);
    
    return String(remainder + "." + month + "." + year);
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    String msg;
    for (byte i = 0; i < length; i++)
    {
        char tmp = char(payload[i]);
        msg += tmp;
    }

    if (strcmp(topic, ISGOOD_TOPIC_SCANNEDAT_PUBLISH) == 0)
    {
        logger.log(Logger::LOG_COMPONENT_MQTT, Logger::LOG_EVENT_INFO, "New callback for topic " + String(ISGOOD_TOPIC_SCANNEDAT_PUBLISH));
        String date = getDateFromJSON(msg);
        drawShowScannedAt(date);
    }
    else
    {
        logger.log(Logger::LOG_COMPONENT_MQTT, Logger::LOG_EVENT_WARN, "Unknown topic " + String(topic));
    }
}

void setup()
{
    // Setup serial monitor
    Serial.begin(9600);

    // Setup display
    if (oledDisplay.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        oledDisplay.clearDisplay();
        oledDisplay.setTextSize(1);
        oledDisplay.setTextColor(WHITE);
        oledDisplay.setCursor(0, 0);
        oledDisplay.println("OLED INIT COMPLETE");
        oledDisplay.display();
        delay(1000);
        logger.log(Logger::LOG_COMPONENT_DISPLAY, Logger::LOG_EVENT_INFO, "Display active");
    }
    else
    {
        logger.log(Logger::LOG_COMPONENT_DISPLAY, Logger::LOG_EVENT_ERROR, "Setting up display failed");
    }

    // Setup joystich switch
    pinMode(JOYSTICK_SWITCH_PIN, INPUT_PULLUP);
    logger.log(Logger::LOG_COMPONENT_JOYSTICK, Logger::LOG_EVENT_INFO, "Joystick switch is ready");

    // Setup joystick axes
    analogMux.begin();
    analogMux.setGain(GAIN_ONE);

    // Setup external monitor
    logger.log(Logger::LOG_COMPONENT_MAIN, Logger::LOG_EVENT_INFO, "Setup external serial monitor");
    scannerSerial.begin(9600);

    // Check if scanner should be reset
    if (RESET_SCANNER)
    {
        logger.log(Logger::LOG_COMPONENT_MAIN, Logger::LOG_EVENT_WARN, "Reset all settings");
        scanner.resetSettings();
        logger.log(Logger::LOG_COMPONENT_MAIN, Logger::LOG_EVENT_INFO, "Set up scanner");
        setupScanner();
    }

    // Query current scanner config
    logger.log(Logger::LOG_COMPONENT_MAIN, Logger::LOG_EVENT_INFO, "Query scanner config");
    scanner.queryConfiguration();

    // Setup wifi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        logger.log(Logger::LOG_COMPONENT_WIFI, Logger::LOG_EVENT_INFO, "Connecting to Wifi ...");
    }
    logger.log(Logger::LOG_COMPONENT_WIFI, Logger::LOG_EVENT_INFO, "Connected to Wifi with IP " + WiFi.localIP().toString());

    // Setup dns discovery
    MDNS.begin(OTA_HOSTNAME);
    MDNS.addService("http", "tcp", 80);
    // Setup http update server with credentials
    httpUpdater.setup(&httpServer, OTA_USERNAME, OTA_PASSWORD);
    // Add root handler
    httpServer.on("/", handleRoot);
    httpServer.begin();
    logger.log(Logger::LOG_COMPONENT_OTA, Logger::LOG_EVENT_INFO, String("OTA and info page are reachable with MDNS name " + OTA_HOSTNAME));

    // Setup mqtt connection
    mqttClient.setServer(MQTT_BROKER_IP, MQTT_BROKER_PORT);
    mqttClient.setCallback(mqttCallback);

    logger.log(Logger::LOG_COMPONENT_MAIN, Logger::LOG_EVENT_INFO, "Leave setup()");
}

void loop()
{
    if (!mqttClient.connected())
    {
        while (!mqttClient.connected())
        {
            logger.log(Logger::LOG_COMPONENT_MQTT, Logger::LOG_EVENT_WARN, "Trying to connect to mqtt broker ...");
            mqttClient.connect("isgood", MQTT_BROKER_USERNAME, MQTT_BROKER_PASSWORD);
            delay(MQTT_DELAY);

            if (mqttClient.connected())
            {
                logger.log(Logger::LOG_COMPONENT_MQTT, Logger::LOG_EVENT_INFO, "Connected to mqtt broker");
                logger.log(Logger::LOG_COMPONENT_MQTT, Logger::LOG_EVENT_INFO, String("Subscribing to topic '" + String(ISGOOD_TOPIC_SCANNEDAT_PUBLISH) + "'"));
                mqttClient.subscribe(ISGOOD_TOPIC_SCANNEDAT_PUBLISH);
                break;
            }
        }
    }

    mqttClient.loop();
    mainLoop();
    httpServer.handleClient();
    MDNS.update();
}