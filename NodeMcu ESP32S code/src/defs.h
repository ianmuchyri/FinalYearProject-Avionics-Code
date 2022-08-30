#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

#define DEBUG 1
#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#define debugf(x, y) Serial.printf(x, y)
#else
#define debug(x)
#define debugln(x)
#define debugf(x, y)
#endif

#define SEA_LEVEL_PRESSURE 102400

// Timing delays
#define SETUP_DELAY 5000

#define SHORT_DELAY 10

#define BAUD_RATE 115200


// Pin to start ejection charge
#define EJECTION_PIN 4


const BaseType_t pro_cpu = 0;
const BaseType_t app_cpu = 1;

const char *ssid = "unknown-network";
const char *password = "4321,dcba";

// MQTT Broker IP address
const char *mqtt_server = "192.168.4.4";

const int MQQT_PORT = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

extern float BASE_ALTITUDE;
extern float MAX_ALTITUDE;

const int PRE_FLIGHT_GROUND_STATE = 0;
const int POWERED_FLIGHT_STATE = 1;
const int COASTING_STATE = 2;
const int BALLISTIC_DESCENT_STATE = 3;
const int CHUTE_DESCENT_STATE = 4;
const int POST_FLIGHT_GROUND_STATE = 5;

const int GROUND_STATE_DISPLACEMENT = 20;
const int BELOW_APOGEE_LEVEL_DISPLACEMENT = 20;

// This struct is used to save all our datapoints.
// It includes rocket altitude, accelerations in the x, y and z directions
// Gryroscope values in the x, y and z direcion
// filtered altitude, velocity and acceleration
// GPS longitude, laltitude and altitude and state
struct Data
{
    uint64_t timeStamp;
    float altitude;
    float ax;
    float ay;
    float az;
    float gx;
    float gy;
    float gz;
    float filtered_s;
    float filtered_v;
    float filtered_a;
    int state;
    float temperature;
  
};
// SensorReadings contains the measurement we are getting
// from the sensors bmp and mpu
struct SensorReadings
{
    float altitude;
    float ax;
    float ay;
    float az;
    float gx;
    float gy;
    float gz;
    float temperature;
};

// FilteredValues contains filtered values from the kalman filter
struct FilteredValues
{
    float displacement;
    float velocity;
    float acceleration;
};

#endif