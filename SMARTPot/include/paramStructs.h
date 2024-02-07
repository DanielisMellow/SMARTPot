#include <Arduino.h>

struct x_Data {
  int adc_raw;
  bool soaking = false;
  bool auton = false;
};
typedef struct x_Data xData;

struct INTERFACE_PARAMS {
  const char *OUT = "output";
  const char *STATE = "state";
};
typedef struct INTERFACE_PARAMS IPARAMS;

// Search for parameter in HTTP POST request
struct HTML_WIFI_PARAMS {
  const char *SSID = "ssid";
  const char *PASSWORD = "pass";
  const char *IP = "ip";
  const char *GATEWAY = "gateway";
};
typedef struct HTML_WIFI_PARAMS HTML_WIFI_PARAMS;

// Variables to save values from HTML form
typedef struct USER_WIFI_DATA {
  String ssid;
  String pass;
  String ip;
  String gateway;
} USER_WIFI_DATA;

// File paths to save input values permanently
struct FILE_PATHS {
  const char *ssidPath = "/ssid.txt";
  const char *passPath = "/pass.txt";
  const char *ipPath = "/ip.txt";
  const char *gatewayPath = "/gateway.txt";
};
typedef struct FILE_PATHS FILE_PATHS;
