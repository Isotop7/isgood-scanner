#include <Arduino.h>
#include <SoftwareSerial.h>
#include <MHET_Live_Barcode_Scanner.h>
#include <CommandResponse.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>

#include "Settings.h"

const bool RESET_SCANNER = false;
const bool ENABLE_DEBUG = false;
const String FIRMWARE_VERSION = "0.1.1";

const String LOG_EVENT_INFO = "INFO";
const String LOG_EVENT_WARN = "WARNING";
const String LOG_EVENT_ERROR = "ERROR";

const String LOG_COMPONENT_MAIN = "ESP8266";
const String LOG_COMPONENT_SCANNER = "SCANNER";
const String LOG_COMPONENT_CONFIG = "CONFIGURATION";
const String LOG_COMPONENT_OTA = "OTA";
const String LOG_COMPONENT_WIFI = "WIFI";
const String LOG_COMPONENT_MQTT = "MQTT";

SoftwareSerial scannerSerial(SCANNER_RX_PIN, SCANNER_TX_PIN);
MHET_Live_Barcode_Scanner scanner(&scannerSerial);
WiFiClient espClient;
PubSubClient mqttClient(espClient);
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

void writeLog(String component, String event, String message)
{
  Serial.println("+ <" + component + "> | [" + event + "] : " + message);
}

void applyCommand(char command[])
{
  writeLog(LOG_COMPONENT_CONFIG, LOG_EVENT_INFO, "Sending command -> " + String(command));

  scannerSerial.write(command);
  delay(SCANNER_SERIAL_BUFFER_TIMEOUT);
  if (scannerSerial.available())
  {
    char response = scannerSerial.read();
    writeLog(LOG_COMPONENT_CONFIG, LOG_EVENT_INFO, "Response:\t" + String(response));

    if (response == CommandResponse::ACK)
    {
      writeLog(LOG_COMPONENT_CONFIG, LOG_EVENT_INFO, "Command successful");
    }
    else if (response == CommandResponse::ENQ)
    {
      writeLog(LOG_COMPONENT_CONFIG, LOG_EVENT_WARN, "Invalid command");
    }
    else
    {
      writeLog(LOG_COMPONENT_CONFIG, LOG_EVENT_WARN, "Unknown response -> " + String(response));
    }
  }
  delay(SCANNER_SERIAL_BUFFER_TIMEOUT);
}

void sendCommand(char command[])
{
  writeLog(LOG_COMPONENT_CONFIG, LOG_EVENT_INFO, "Sending command -> " + String(command));
  scannerSerial.write(command);
}

void resetAllSettings()
{
  applyCommand("~M00910001.\n");
  applyCommand("~MA5F01B2C.\n");
  applyCommand("~MA5F0D201.\n");
  applyCommand("~M00510000.\n");
  applyCommand("~MA5F0506A.\n");
  applyCommand("~M00910000.\n");
}

void setupScanner()
{
  applyCommand("~M00910001.\n");
  
  applyCommand("~M00510000.\n");
  applyCommand("~M00210000.\n");
  applyCommand("~M00FA0002.\n");
  applyCommand("~M00FB0003.\n");
  applyCommand("~M00FC0002.\n");

  applyCommand("~MA5F0506A.\n");
  applyCommand("~M00910000.\n");
}

void enableOutput()
{
  applyCommand("~M00910001.");
  applyCommand("~M00510000.");
  applyCommand("~MA5F0506A.");
  applyCommand("~M00910000.");
}

void showConfig()
{
  String config;
  // Init config mode
  applyCommand("~M00910001.\n");
  // Query config
  sendCommand("~QFA50.\n");
  // Wait for data to arrive in buffer
  delay(SCANNER_SERIAL_BUFFER_TIMEOUT);
  
  // Read data from buffer
  while (scannerSerial.available())
  {
    char key = scannerSerial.read();
    // Add key to config
    config += key;

    // If we cant find data, wait for the serial timeout to be sure no more data has arrived while emptying buffer
    if (scannerSerial.available() == 0)
    {
      delay(SCANNER_SERIAL_BUFFER_TIMEOUT);
    }
  }

  int configSize = config.length();
  writeLog(LOG_COMPONENT_CONFIG, LOG_EVENT_INFO, "Size of config data: " + configSize);

  char rawConfig[configSize];
  config.toCharArray(rawConfig, configSize, 1);
  scanner.readConfig(rawConfig, configSize);
  std::vector<Command> configuration = scanner.getConfigurationInstance();

  int commandElements = 0;
  for (const Command& obj : configuration) {
      String message = "OptionCode -> ";
      message += obj.getOptionCode();
      message += "; Value -> ";
      message += String(obj.getValue(), HEX);
      writeLog(LOG_COMPONENT_CONFIG, LOG_EVENT_INFO, message);
      commandElements++;
  }
  writeLog(LOG_COMPONENT_CONFIG, LOG_EVENT_INFO, "Number of configuration commands: " + commandElements);

  //Serial.println(config);
  applyCommand("~M00910000.\n");
}

String getNextBarcode()
{
  String barcode = "";
  char receivedChar;

  while (scannerSerial.available())
  {
    receivedChar = scannerSerial.read();
    if (receivedChar == CommandResponse::EOL) {
      writeLog(LOG_COMPONENT_SCANNER, LOG_EVENT_INFO, "New barcode scanned -> " + barcode);
      break; // Exit the loop if line feed character is received
    }
    barcode += receivedChar;

    // If we cant find data, wait for the serial timeout to be sure no more data has arrived while emptying buffer
    if (scannerSerial.available() == 0)
    {
      delay(SCANNER_SERIAL_BUFFER_TIMEOUT);
    }
  }
  return barcode;
}

/* HTTP */

void handleRoot() {
  String response = F("<!DOCTYPE html>\n"
              "<html lang='en'>\n"
              "<head>\n"
              "<title>isgood-scanner</title>\n"
              "</head>\n"
              "<link rel=\"stylesheet\" href=\"https://unpkg.com/sakura.css/css/sakura.css\" type=\"text/css\">\n" 
              "<body>\n"
              "<h1>isgood-scanner</h1>\r\n");
  response = response + "<h2>firmware version: " + FIRMWARE_VERSION + "</h2>\r\n";
  response = response + "</body></html>";
  httpServer.send(200, "text/html", response);
}

void setup() {
  // Setup serial monitors
  Serial.begin(9600);
  delay(1000);
  writeLog(LOG_COMPONENT_MAIN, LOG_EVENT_INFO, "Setup external serial monitor");
  scannerSerial.begin(9600);

  // Check if scanner should be reset
  if (RESET_SCANNER)
  {
    writeLog(LOG_COMPONENT_MAIN, LOG_EVENT_WARN, "Reset all settings");
    resetAllSettings();
    writeLog(LOG_COMPONENT_MAIN, LOG_EVENT_INFO, "Set up scanner");
    setupScanner();
  }

  // Show current config
  writeLog(LOG_COMPONENT_MAIN, LOG_EVENT_INFO, "Show config");
  showConfig();

  // Setup wifi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
      delay(500);
      writeLog(LOG_COMPONENT_WIFI, LOG_EVENT_INFO, "Connecting ...");
  }
  writeLog(LOG_COMPONENT_WIFI, LOG_EVENT_INFO, "Connected to Wifi with IP " + WiFi.localIP().toString());

  // Setup dns discovery
  MDNS.begin(OTA_HOSTNAME);
  MDNS.addService("http", "tcp", 80);
  // Setup http update server with credentials
  httpUpdater.setup(&httpServer, OTA_USERNAME, OTA_PASSWORD);
  // Add root handler
  httpServer.on("/", handleRoot);
  httpServer.begin();
  writeLog(LOG_COMPONENT_OTA, LOG_EVENT_INFO, OTA_HOSTNAME);

  // Setup mqtt connection
  mqttClient.setServer(MQTT_BROKER_IP, MQTT_BROKER_PORT);
  //client.setCallback(callback);

  writeLog(LOG_COMPONENT_MAIN, LOG_EVENT_INFO, "Leave setup()");
}

void loop() {
  String nextBarcode = getNextBarcode();
  if (!mqttClient.connected()) {
    while (!mqttClient.connected()) {
      writeLog(LOG_COMPONENT_MQTT, LOG_EVENT_WARN, "Trying to connect to mqtt broker ...");
      mqttClient.connect("isgood", MQTT_BROKER_USERNAME, MQTT_BROKER_PASSWORD);
      delay(MQTT_DELAY);
    }
  }
  if (nextBarcode.length() > 0)
  {
    writeLog(LOG_COMPONENT_MQTT, LOG_EVENT_INFO, "Publish new barcode " + nextBarcode);
    mqttClient.publish(ISGOOD_TOPIC_BARCODE, nextBarcode.c_str());
  }

  mqttClient.loop();
  httpServer.handleClient();
  MDNS.update();
}