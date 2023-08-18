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

#include <MHET_Live_Barcode_Scanner.h>
#include <Command.h>
#include <Product.h>
#include <Logger.h>

#include "Settings.h"

const bool RESET_SCANNER = false;
const bool ENABLE_DEBUG = false;
const String FIRMWARE_VERSION = "0.3.2";

Adafruit_SSD1306 oledDisplay(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET_PIN);
Adafruit_ADS1115 analogMux;
Logger logger(oledDisplay);
SoftwareSerial scannerSerial(SCANNER_RX_PIN, SCANNER_TX_PIN);
MHET_Live_Barcode_Scanner scanner(&scannerSerial, SCANNER_SERIAL_BUFFER_TIMEOUT, logger);
WiFiClient espClient;
PubSubClient mqttClient(espClient);
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

const String menuItems[] = {"Scan Barcode", "Show ScannedAt", "Remove product", "Show log"};
int8_t currentMenuItemIndex = 0;
int8_t activeSubMenuIndex = 0;
bool displayAvailable = true;
bool joystickButtonLatch = false;
bool joystickDownLatch = false;
bool joystickLeftLatch = false;
bool joystickUpLatch = false;
bool joystickRightLatch = false;
bool menuRefreshPending = true;
bool menuItemRefreshPending;

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
    for (const Command& obj : configuration) {
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

void handleRoot() {
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
      oledDisplay.println("Waiting for new barcode ...");
      oledDisplay.display();

      menuItemRefreshPending = false;
  }

  Product product = Product(scanner.getNextBarcode());
  if (product.isValid())
  {
      logger.log(Logger::LOG_COMPONENT_MQTT, Logger::LOG_EVENT_INFO, "Publish new barcode " + product.getBarcode());
      mqttClient.publish(ISGOOD_TOPIC_BARCODE, product.toJSON().c_str());
  }
}

void drawMenuShowScannedAt()
{
    if (menuItemRefreshPending)
    {
        logger.log(Logger::LOG_COMPONENT_MAIN, Logger::LOG_EVENT_ERROR, "Not implemented!");
        menuItemRefreshPending = false;
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


void selectNextItem()
{
    currentMenuItemIndex = currentMenuItemIndex + 1;
    if (currentMenuItemIndex >= menuItems->length())
    {
        currentMenuItemIndex = 0;
    }
}

void selectPreviousItem()
{
    currentMenuItemIndex = currentMenuItemIndex - 1;
    if (currentMenuItemIndex < 0)
    {
        currentMenuItemIndex = menuItems->length() - 1;
    }
}

void executeSelectedItem()
{
    activeSubMenuIndex = currentMenuItemIndex + 1;
    menuItemRefreshPending = true;
}

void mainLoop()
{
  if (activeSubMenuIndex == 0)
    {
        // Reset latches
        menuItemRefreshPending = true;
        joystickLeftLatch = false;
        // Read joystick
        int adc0 = analogMux.readADC_SingleEnded(0);
        long xAxisValue = map(adc0,0,32768,0,1000);

        // Detect input down
        if (xAxisValue < 300 && joystickDownLatch == false)
        {
            joystickDownLatch = true;
            selectNextItem();
            menuRefreshPending = true;;
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
            menuRefreshPending = true;;
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

            for (u_int8_t i = 0; i < menuItems->length(); ++i) 
            {
                String item;
                if (i == currentMenuItemIndex) 
                {
                    item += ">\t";
                } else 
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
        long yAxisValue = map(adc1,0,32768,0,1000);

        if (yAxisValue < 300 && joystickLeftLatch == false)
        {
            joystickLeftLatch = true;
            activeSubMenuIndex = 0;
            menuRefreshPending = true;
        }
        else 
        {
            switch (activeSubMenuIndex)
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
                    logger.log(Logger::LOG_COMPONENT_MAIN, Logger::LOG_EVENT_WARN, "Invalid menu index");
            }
        }
    }
}

void setup() {
  // Setup serial monitor
  Serial.begin(9600);
  
  // Setup display
  if(oledDisplay.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
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

  // Show current config
  //logger.log(Logger::LOG_COMPONENT_MAIN, Logger::LOG_EVENT_INFO, "Show config");
  //showConfig();

  // Setup wifi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
      delay(500);
      logger.log(Logger::LOG_COMPONENT_WIFI, Logger::LOG_EVENT_INFO, "Connecting ...");
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
  logger.log(Logger::LOG_COMPONENT_OTA, Logger::LOG_EVENT_INFO, OTA_HOSTNAME);

  // Setup mqtt connection
  mqttClient.setServer(MQTT_BROKER_IP, MQTT_BROKER_PORT);
  //client.setCallback(callback);

  logger.log(Logger::LOG_COMPONENT_MAIN, Logger::LOG_EVENT_INFO, "Leave setup()");
}

void loop() {
  if (!mqttClient.connected()) {
    while (!mqttClient.connected()) {
      logger.log(Logger::LOG_COMPONENT_MQTT, Logger::LOG_EVENT_WARN, "Trying to connect to mqtt broker ...");
      mqttClient.connect("isgood", MQTT_BROKER_USERNAME, MQTT_BROKER_PASSWORD);
      delay(MQTT_DELAY);

      if (mqttClient.connected())
      {
        logger.log(Logger::LOG_COMPONENT_MQTT, Logger::LOG_EVENT_INFO, "Connected to mqtt broker");
        break;
      }
    }
  }
  
  mqttClient.loop();
  mainLoop();
  httpServer.handleClient();
  MDNS.update();
}