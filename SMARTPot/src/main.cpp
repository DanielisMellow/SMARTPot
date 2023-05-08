#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Wire.h>

// Macros For Debugging
#define DEBUG 0
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

// Uncomment the type of sensor in use:
// #define DHTTYPE DHT22 // DHT 22 (AM2302)
Adafruit_BME280 bme;

// STRUCTURE convert this to a static structure
xQueueHandle duty_queue;
typedef struct
{
  int adc_raw;
  bool soaking = false;
  bool auton = false;

} xData;

static xData ADC_READINGS;

const char *PARAM_INPUT_5 = "output";
const char *PARAM_INPUT_6 = "state";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Search for parameter in HTTP POST request
const char *PARAM_INPUT_1 = "ssid";
const char *PARAM_INPUT_2 = "pass";
const char *PARAM_INPUT_3 = "ip";
const char *PARAM_INPUT_4 = "gateway";

// Variables to save values from HTML form
String ssid;
String pass;
String ip;
String gateway;

// File paths to save input values permanently
const char *ssidPath = "/ssid.txt";
const char *passPath = "/pass.txt";
const char *ipPath = "/ip.txt";
const char *gatewayPath = "/gateway.txt";

IPAddress localIP;
// IPAddress localIP(192, 168, 1, 200); // hardcoded

// Set your Gateway IP address
IPAddress localGateway;
// IPAddress localGateway(192, 168, 1, 1); //hardcoded
IPAddress subnet(255, 255, 0, 0);

// Timer variables
unsigned long previousMillis = 0;
const long interval = 10000; // interval to wait for Wi-Fi connection (milliseconds)

// Initialize SPIFFS
void initSPIFFS()
{
  if (!SPIFFS.begin(true))
  {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

// Read File from SPIFFS
String readFile(fs::FS &fs, const char *path)
{
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory())
  {
    Serial.println("- failed to open file for reading");
    return String();
  }

  String fileContent;
  while (file.available())
  {
    fileContent = file.readStringUntil('\n');
    break;
  }
  return fileContent;
}

// Write file to SPIFFS
void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    Serial.println("- file written");
  }
  else
  {
    Serial.println("- write failed");
  }
}

// Initialize WiFi
bool initWiFi()
{

  if (ssid == "" || ip == "")
  {
    Serial.println("Undefined SSID or IP address.");
    return false;
  }

  WiFi.mode(WIFI_STA);

  localIP.fromString(ip.c_str());
  localGateway.fromString(gateway.c_str());

  if (!WiFi.config(localIP, localGateway, subnet))
  {
    Serial.println("STA Failed to configure");
    return false;
  }

  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.println("Connecting to WiFi...");

  unsigned long currentMillis = millis();
  previousMillis = currentMillis;

  while (WiFi.status() != WL_CONNECTED)
  {
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
    {
      Serial.println("Failed to connect.");
      return false;
    }
  }

  Serial.println(WiFi.localIP());
  return true;
}

// FUNCTIONS TO UPDATE STATS
String readBME280Temperature()
{
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Celsius (the default)
  float t = bme.readTemperature();
  // t = t * 1.8 + 32;
  //  Read temperature as Fahrenheit (isFahrenheit = true)
  //  float t = dht.readTemperature(true);
  //  Check if any reads failed and exit early (to try again).
  if (isnan(t))
  {
    debugln("Failed to read from BME280 sensor!");
    return "--";
  }
  else
  {
    debugln(t);
    return String(t);
  }
}

String readBME280Humidity()
{
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = bme.readHumidity();
  if (isnan(h))
  {
    debugln("Failed to read from BME280 sensor!");
    return "--";
  }
  else
  {
    debugln(h);
    return String(h);
  }
}

void TaskAnalogReadA6(void *pvParameters) // This is a task.
{
  (void)pvParameters;

  while (1)
  {
    ADC_READINGS.adc_raw = 0;
    int nSamples = 15;
    for (int i = 0; i < nSamples; i++)
    {
      ADC_READINGS.adc_raw += analogRead(A6);
    }

    ADC_READINGS.adc_raw /= nSamples;

    BaseType_t xStatus;
    const TickType_t xTicksToWait = pdMS_TO_TICKS(2000);
    xStatus = xQueueSendToBack(duty_queue, &ADC_READINGS, xTicksToWait);
    // debugln(ADC_READINGS.adc_raw);
    if (xStatus != pdPASS)
    {
      debugln("Could not send to the queue.\r\n");
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void TaskRelay(void *pvParameters) // This is a task.
{
  (void)pvParameters;
  xData ADC_READINGS;
  while (1)
  {
    if (xQueueReceive(duty_queue, &ADC_READINGS, portMAX_DELAY))
    {
      // debug("ADC READING RECEIVED BY RELAY TASK: ");
      debug("ADC Raw: ");
      debug(ADC_READINGS.adc_raw);
      debug(" Water: ");
      debug(ADC_READINGS.soaking);
      debug(" Autonomous: ");
      debugln(ADC_READINGS.auton);

      if (ADC_READINGS.auton != 1) // Autonomous mode is not triggered
      {
        digitalWrite(GREEN, LOW);
        if (ADC_READINGS.soaking != false) // Manual water is toggled
        {
          // TURN ON WATER PUMP and Blue LED indicator
          digitalWrite(13, HIGH);
          digitalWrite(BLUE, HIGH);
        }
        else
        {
          // TURN OFF WATER PUMP and LED indicator
          digitalWrite(13, LOW);
          digitalWrite(BLUE, LOW);
        }
      }
      else
      {
        // Toggle Autonomous Green LED indicator (Take ADC Readings Into consideration)
        digitalWrite(GREEN, HIGH);

        // Soil is Dry: Requires water
        if (ADC_READINGS.adc_raw > 1650)
        {
          digitalWrite(13, HIGH);
          digitalWrite(BLUE, HIGH);
        }
        // Soil has sufficient amounts of water: turned water pump off
        else if (ADC_READINGS.adc_raw < 1600)
        {
          digitalWrite(13, LOW);
          digitalWrite(BLUE, LOW);
        }
      }
    }
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);

  pinMode(BLUE, OUTPUT);
  digitalWrite(BLUE, LOW);

  pinMode(GREEN, OUTPUT);
  digitalWrite(GREEN, LOW);

  bool status = bme.begin(0x76);
  if (!status)
  {
    debugln("Could not find valid BME280 sensor");
    while (1)
      ;
  }

  initSPIFFS();

  // Load values saved in SPIFFS
  ssid = readFile(SPIFFS, ssidPath);
  pass = readFile(SPIFFS, passPath);
  ip = readFile(SPIFFS, ipPath);
  gateway = readFile(SPIFFS, gatewayPath);
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(ip);
  Serial.println(gateway);

  if (initWiFi())
  {
    // Create A Queue
    duty_queue = xQueueCreate(10, sizeof(xData));

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/index.html"); });

    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/style.css", "text/css"); });

    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/script.js", "text/javascript"); });

    server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/plain", readBME280Temperature().c_str()); });
    server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/plain", readBME280Humidity().c_str()); });

    xTaskCreatePinnedToCore(
        TaskAnalogReadA6, "AnalogReadA6", 2048 // Stack size
        ,
        NULL, 1 // Priority
        ,
        NULL, 1);

    // Now set up two tasks to run independently.
    xTaskCreatePinnedToCore(
        TaskRelay, "TaskRelay" // A name just for humans
        ,
        1024 // This stack size can be checked & adjusted by reading the Stack Highwater
        ,
        NULL, 1 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,
        NULL, 1);

    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request)
              {
              String inputMessage1;
              String inputMessage2;
              // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
              if (request->hasParam(PARAM_INPUT_5) && request->hasParam(PARAM_INPUT_6))
              {
                inputMessage1 = request->getParam(PARAM_INPUT_5)->value();
                inputMessage2 = request->getParam(PARAM_INPUT_6)->value();
                //digitalWrite(inputMessage1.toInt(), inputMessage2.toInt());

                if (inputMessage1.toInt() == RELAY)
                {
                  ADC_READINGS.soaking = inputMessage2.toInt();
                }
                if (inputMessage1.toInt() == AUTO)
                {
                  ADC_READINGS.auton = inputMessage2.toInt();
                }
              }
              else
              {
                inputMessage1 = "No message sent";
                inputMessage2 = "No message sent";
              }
              debug("Toggle Switch: ");
              debug(inputMessage1);
              debug(" - Set to: ");
              debugln(inputMessage2);
              request->send(200, "text/plain", "OK"); });

    // Start server
    server.begin();
  }
  else
  {
    // Connect to Wi-Fi network with SSID and password
    Serial.println("Setting AP (Access Point)");
    // NULL sets an open Access Point
    WiFi.softAP("ESP-WIFI-MANAGER", NULL);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/wifimanager.html", "text/html"); });

    server.serveStatic("/", SPIFFS, "/");

    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request)
              {
      int params = request->params();
      for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1) {
            ssid = p->value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(ssid);
            // Write file to save value
            writeFile(SPIFFS, ssidPath, ssid.c_str());
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_2) {
            pass = p->value().c_str();
            Serial.print("Password set to: ");
            Serial.println(pass);
            // Write file to save value
            writeFile(SPIFFS, passPath, pass.c_str());
          }
          // HTTP POST ip value
          if (p->name() == PARAM_INPUT_3) {
            ip = p->value().c_str();
            Serial.print("IP Address set to: ");
            Serial.println(ip);
            // Write file to save value
            writeFile(SPIFFS, ipPath, ip.c_str());
          }
          // HTTP POST gateway value
          if (p->name() == PARAM_INPUT_4) {
            gateway = p->value().c_str();
            Serial.print("Gateway set to: ");
            Serial.println(gateway);
            // Write file to save value
            writeFile(SPIFFS, gatewayPath, gateway.c_str());
          }
          //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }
      request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
      delay(3000);
      ESP.restart(); });
    server.begin();
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
}