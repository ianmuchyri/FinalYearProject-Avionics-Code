#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "defs.h"
#include "readsensors.h"

void ejection();
void ejectionTimerCallback(TimerHandle_t ejectionTimerHandle);

// formats data that we are going to save to SD card
// We save all the data points we are collecting
struct Data formart_data(SensorReadings readings, FilteredValues filtered_values)
{
 struct Data dt = {0};
  dt.altitude = readings.altitude;
  dt.temperature = readings.temperature;
  dt.ax = readings.ax;
  dt.ay = readings.ay;
  dt.az = readings.az;
  dt.gx = readings.gx;
  dt.gy = readings.gy;
  dt.gz = readings.gz;
  dt.filtered_s = filtered_values.displacement;
  dt.filtered_a = filtered_values.acceleration;
  dt.filtered_v = filtered_values.velocity;
  
  return dt;
}
struct SendValues formart_send_data(Data readings)
{
  struct SendValues sv = {0};
  sv.altitude = readings.altitude;
  sv.state = readings.state;
  sv.timeStamp = readings.timeStamp;
  sv.latitude = readings.latitude;
  sv.longitude = readings.longitude;
  return sv;
}

// get_base_altitude Finds the average of the current altitude from 100 readings
float get_base_altitude()
{
  float altitude = 0;
  SensorReadings readings;
  for (int i = 0; i < 100; i++)
  {
    readings = get_readings();
    altitude = altitude + readings.altitude;
  }
  altitude = altitude / 100.0;
  debugf("Base Altitude is %.3f\n", altitude);
  return altitude;
}
#endif