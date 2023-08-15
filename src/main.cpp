#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>

#include <MHET_Live_Barcode_Scanner.h>
#include <Command.h>
#include <Product.h>
#include <Logger.h>

#include "Settings.h"

const bool RESET_SCANNER = false;
const bool ENABLE_DEBUG = false;
const String FIRMWARE_VERSION = "0.1.6";

Logger logger;
SoftwareSerial scannerSerial(SCANNER_RX_PIN, SCANNER_TX_PIN);
MHET_Live_Barcode_Scanner scanner(&scannerSerial, SCANNER_SERIAL_BUFFER_TIMEOUT, logger);
WiFiClient espClient;
PubSubClient mqttClient(espClient);
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

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

void setup() {
  // Setup serial monitors
  Serial.begin(9600);
  delay(1000);
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
  logger.log(Logger::LOG_COMPONENT_MAIN, Logger::LOG_EVENT_INFO, "Show config");
  showConfig();

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
  Product product = Product(scanner.getNextBarcode());
  if (!mqttClient.connected()) {
    while (!mqttClient.connected()) {
      logger.log(Logger::LOG_COMPONENT_MQTT, Logger::LOG_EVENT_WARN, "Trying to connect to mqtt broker ...");
      mqttClient.connect("isgood", MQTT_BROKER_USERNAME, MQTT_BROKER_PASSWORD);
      delay(MQTT_DELAY);
    }
  }
  if (product.isValid())
  {
    logger.log(Logger::LOG_COMPONENT_MQTT, Logger::LOG_EVENT_INFO, "Publish new barcode " + product.getBarcode());
    mqttClient.publish(ISGOOD_TOPIC_BARCODE, product.toJSON().c_str());
  }

  mqttClient.loop();
  httpServer.handleClient();
  MDNS.update();
}