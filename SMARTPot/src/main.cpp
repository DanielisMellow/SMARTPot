#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
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

// Uncomment the type of sensor in use:
// #define DHTTYPE DHT22 // DHT 22 (AM2302)

Adafruit_BME280 bme;

// Network credentials
const char *ssid = "Daniels_WiFi";
const char *password = "Rebel1x1";

// STRUCTURE convert this to a static structure
xQueueHandle duty_queue;
typedef struct
{
  int adc_raw;
  bool soaking = false;
  bool auton = false;

} xData;

xData ADC_READINGS;

const char *PARAM_INPUT_1 = "output";
const char *PARAM_INPUT_2 = "state";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

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
// Improve Function logic
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
      if (ADC_READINGS.auton == 1) // Autonomous mode is activated, relay is on/off by ADC values
      {
        digitalWrite(GREEN, HIGH);
        // Soil is Dry and Requires water
        if (ADC_READINGS.adc_raw > 1650)
        {
          digitalWrite(13, HIGH);
          digitalWrite(BLUE, HIGH);
        }

        // Soil has sufficient amounts of water the water pump should be turned off
        else if (ADC_READINGS.adc_raw < 1600)
        {
          digitalWrite(13, LOW);
          digitalWrite(BLUE, LOW);
        }
      }
      else // Autonomous mode is off, relay is controlled by water button
      {
        if (ADC_READINGS.soaking == 1)
        {
          // TURN ON WATER PUMP
          digitalWrite(13, HIGH);
          digitalWrite(BLUE, HIGH);
        }
        else if (ADC_READINGS.soaking == 0)
        {
          digitalWrite(13, LOW);
          // TURN OFF WATER PUMP
          digitalWrite(BLUE, LOW);
        }

        if (ADC_READINGS.auton == 0)
        {
          // digitalWrite(13, LOW);
          digitalWrite(GREEN, LOW);
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

  bool status;
  status = bme.begin(0x76);
  if (!status)
  {
    debugln("Could not find valid BME280 sensor");
    while (1)
      ;
  }

  // Create A Queue
  duty_queue = xQueueCreate(10, sizeof(xData));

  // Initialize SPIFFS
  if (!SPIFFS.begin())
  {
    debugln("An Error has occurred while mounting SPIFFS");
    return;
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    debugln("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  debugln(WiFi.localIP());

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
              if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2))
              {
                inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
                inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
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

void loop()
{
  // put your main code here, to run repeatedly:
}