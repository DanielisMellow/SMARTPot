#include "paramStructs.h"
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <Wire.h>

// Macros For Debugging
#define DEBUG 1
#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

// Define
#define RELAY 13
#define AUTO 0
#define BLUE 12
#define GREEN 14

xQueueHandle duty_queue;
static xData ADC_READINGS;
static IPARAMS interfaceParams;
static HTML_WIFI_PARAMS HTMLWifiInputs;
static USER_WIFI_DATA UserWifiData;
static FILE_PATHS filePaths;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
// Initialize BME sensor
Adafruit_BME280 bme;

// IPAddress localIP(192, 168, 1, 200); // hardcoded
IPAddress localIP;
// Set your Gateway IP address
IPAddress localGateway;
// IPAddress localGateway(192, 168, 1, 1); //hardcoded
IPAddress subnet(255, 255, 0, 0);

// Timer variables
unsigned long previousMillis = 0;
const long interval =
    10000; // interval to wait for Wi-Fi connection (milliseconds)

// Initialize SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    debugln("An error has occurred while mounting SPIFFS");
  }
  debugln("SPIFFS mounted successfully");
}

// Read File from SPIFFS
String readFile(fs::FS &fs, const char *path) {
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory()) {
    debugln("- failed to open file for reading");
    return String();
  }

  String fileContent;
  while (file.available()) {
    fileContent = file.readStringUntil('\n');
    break;
  }
  return fileContent;
}

// Write file to SPIFFS
void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    debugln("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    debugln("- file written");
  } else {
    debugln("- write failed");
  }
}

// Initialize WiFi
bool initWiFi() {

  if (UserWifiData.ssid == "" || UserWifiData.ip == "") {
    debugln("Undefined SSID or IP address.");
    return false;
  }

  WiFi.mode(WIFI_STA);

  localIP.fromString(UserWifiData.ip.c_str());
  localGateway.fromString(UserWifiData.gateway.c_str());

  if (!WiFi.config(localIP, localGateway, subnet)) {
    debugln("STA Failed to configure");
    return false;
  }

  WiFi.begin(UserWifiData.ssid.c_str(), UserWifiData.pass.c_str());
  debugln("Connecting to WiFi...");

  unsigned long currentMillis = millis();
  previousMillis = currentMillis;

  while (WiFi.status() != WL_CONNECTED) {
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      debugln("Failed to connect.");
      return false;
    }
  }
  debugln("Connected to WiFi");
  debugln(WiFi.localIP());
  return true;
}

// FUNCTIONS TO UPDATE STATS
String readBME280Temperature() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Celsius (the default)
  float t = bme.readTemperature();
  // t = t * 1.8 + 32;
  //  Read temperature as Fahrenheit (isFahrenheit = true)
  //  float t = dht.readTemperature(true);
  //  Check if any reads failed and exit early (to try again).
  if (isnan(t)) {
    debugln("Failed to read from BME280 sensor!");
    return "--";
  } else {
    debugln(t);
    return String(t);
  }
}

String readBME280Humidity() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = bme.readHumidity();
  if (isnan(h)) {
    debugln("Failed to read from BME280 sensor!");
    return "--";
  } else {
    debugln(h);
    return String(h);
  }
}

void TaskAnalogReadA6(void *pvParameters) // This is a task.
{
  (void)pvParameters;

  while (1) {
    ADC_READINGS.adc_raw = 0;
    int nSamples = 15;
    for (int i = 0; i < nSamples; i++) {
      ADC_READINGS.adc_raw += analogRead(A6);
    }

    ADC_READINGS.adc_raw /= nSamples;

    BaseType_t xStatus;
    const TickType_t xTicksToWait = pdMS_TO_TICKS(2000);
    xStatus = xQueueSendToBack(duty_queue, &ADC_READINGS, xTicksToWait);
    // debugln(ADC_READINGS.adc_raw);
    if (xStatus != pdPASS) {
      debugln("Could not send to the queue.\r\n");
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void TaskRelay(void *pvParameters) // This is a task.
{
  (void)pvParameters;
  xData ADC_READINGS;
  while (1) {
    if (xQueueReceive(duty_queue, &ADC_READINGS, portMAX_DELAY)) {
      // debug("ADC READING RECEIVED BY RELAY TASK: ");
      debug("ADC Raw: ");
      debug(ADC_READINGS.adc_raw);
      debug(" Water: ");
      debug(ADC_READINGS.soaking);
      debug(" Autonomous: ");
      debugln(ADC_READINGS.auton);

      if (ADC_READINGS.auton != true) // Autonomous mode is not triggered
      {
        digitalWrite(GREEN, LOW);
        if (ADC_READINGS.soaking != false) // Manual water is toggled
        {
          // TURN ON WATER PUMP and Blue LED indicator
          digitalWrite(13, HIGH);
          digitalWrite(BLUE, HIGH);
        } else {
          // TURN OFF WATER PUMP and LED indicator
          digitalWrite(13, LOW);
          digitalWrite(BLUE, LOW);
        }
      } else {
        // Toggle Autonomous Green LED indicator (Take ADC Readings Into
        // consideration)
        digitalWrite(GREEN, HIGH);

        // Soil is Dry: Requires water
        if (ADC_READINGS.adc_raw > 1650) {
          digitalWrite(13, HIGH);
          digitalWrite(BLUE, HIGH);
        }
        // Soil has sufficient amounts of water: turned water pump off
        else if (ADC_READINGS.adc_raw < 1600) {
          digitalWrite(13, LOW);
          digitalWrite(BLUE, LOW);
        }
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);

  pinMode(BLUE, OUTPUT);
  digitalWrite(BLUE, LOW);

  pinMode(GREEN, OUTPUT);
  digitalWrite(GREEN, LOW);

  bool status = bme.begin(0x76);
  if (!status) {
    debugln("Could not find valid BME280 sensor");
    while (1)
      ;
  }

  initSPIFFS();

  // Load values saved in SPIFFS
  UserWifiData.ssid = readFile(SPIFFS, filePaths.ssidPath);
  UserWifiData.pass = readFile(SPIFFS, filePaths.passPath);
  UserWifiData.ip = readFile(SPIFFS, filePaths.ipPath);
  UserWifiData.gateway = readFile(SPIFFS, filePaths.gatewayPath);
  debugln(UserWifiData.ssid);
  debugln(UserWifiData.pass);
  debugln(UserWifiData.ip);
  debugln(UserWifiData.gateway);

  if (initWiFi()) {
    // Create A Queue
    duty_queue = xQueueCreate(10, sizeof(xData));

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/index.html");
    });

    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/style.css", "text/css");
    });

    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/script.js", "text/javascript");
    });

    server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send_P(200, "text/plain", readBME280Temperature().c_str());
    });
    server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send_P(200, "text/plain", readBME280Humidity().c_str());
    });

    xTaskCreatePinnedToCore(TaskAnalogReadA6, "AnalogReadA6", 2048 // Stack size
                            ,
                            NULL, 1 // Priority
                            ,
                            NULL, 1);

    // Now set up two tasks to run independently.
    xTaskCreatePinnedToCore(TaskRelay, "TaskRelay" // A name just for humans
                            ,
                            1024 // This stack size can be checked & adjusted by
                                 // reading the Stack Highwater
                            ,
                            NULL,
                            1 // Priority, with 3 (configMAX_PRIORITIES - 1)
                              // being the highest, and 0 being the lowest.
                            ,
                            NULL, 1);

    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
      String inputMessage1;
      String inputMessage2;
      // GET input1 value on
      // <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
      if (request->hasParam(interfaceParams.OUT) &&
          request->hasParam(interfaceParams.STATE)) {
        inputMessage1 = request->getParam(interfaceParams.OUT)->value();
        inputMessage2 = request->getParam(interfaceParams.STATE)->value();
        // digitalWrite(inputMessage1.toInt(), inputMessage2.toInt());

        if (inputMessage1.toInt() == RELAY) {
          ADC_READINGS.soaking = inputMessage2.toInt();
        }
        if (inputMessage1.toInt() == AUTO) {
          ADC_READINGS.auton = inputMessage2.toInt();
        }
      } else {
        inputMessage1 = "No message sent";
        inputMessage2 = "No message sent";
      }
      debug("Toggle Switch: ");
      debug(inputMessage1);
      debug(" - Set to: ");
      debugln(inputMessage2);
      request->send(200, "text/plain", "OK");
    });

    // Start server
    server.begin();
  } else {
    // Connect to Wi-Fi network with SSID and password
    debugln("Setting AP (Access Point)");
    // NULL sets an open Access Point
    WiFi.softAP("SmartPot", NULL);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    debugln(IP);

    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/wifimanager.html", "text/html");
    });

    server.serveStatic("/", SPIFFS, "/");

    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      for (int i = 0; i < params; i++) {
        AsyncWebParameter *p = request->getParam(i);
        if (p->isPost()) {
          // HTTP POST ssid value
          if (p->name() == HTMLWifiInputs.SSID) {
            UserWifiData.ssid = p->value().c_str();
            Serial.print("SSID set to: ");
            debugln(UserWifiData.ssid);
            // Write file to save value
            writeFile(SPIFFS, filePaths.ssidPath, UserWifiData.ssid.c_str());
          }
          // HTTP POST pass value
          if (p->name() == HTMLWifiInputs.PASSWORD) {
            UserWifiData.pass = p->value().c_str();
            Serial.print("Password set to: ");
            debugln(UserWifiData.pass);
            // Write file to save value
            writeFile(SPIFFS, filePaths.passPath, UserWifiData.pass.c_str());
          }
          // HTTP POST ip value
          if (p->name() == HTMLWifiInputs.IP) {
            UserWifiData.ip = p->value().c_str();
            Serial.print("IP Address set to: ");
            debugln(UserWifiData.ip);
            // Write file to save value
            writeFile(SPIFFS, filePaths.ipPath, UserWifiData.ip.c_str());
          }
          // HTTP POST gateway value
          if (p->name() == HTMLWifiInputs.GATEWAY) {
            UserWifiData.gateway = p->value().c_str();
            Serial.print("Gateway set to: ");
            debugln(UserWifiData.gateway);
            // Write file to save value
            writeFile(SPIFFS, filePaths.gatewayPath,
                      UserWifiData.gateway.c_str());
          }
          // Serial.printf("POST[%s]: %s\n", p->name().c_str(),
          // p->value().c_str());
        }
      }
      request->send(200, "text/plain",
                    "Done. ESP will restart, connect to your router and go to "
                    "IP address: " +
                        UserWifiData.ip);
      delay(3000);
      ESP.restart();
    });
    server.begin();
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}
