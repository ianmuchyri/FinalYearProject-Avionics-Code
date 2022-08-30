#ifndef READSENSORS_H
#define READSENSORS_H

#include <FS.h>
#include <SPI.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "defs.h"
#include <SPI.h>

Adafruit_BMP085 bmp;
Adafruit_MPU6050 mpu;

void init_mpu()
{
    debugln("MPU6050 test!");
    if (!mpu.begin())
    {
        debugln("Could not find a valid MPU6050 sensor, check wiring!");
        while (1)
        {
            //TODO: add beep to notify 
        }
    }
    else
    {
        debugln("MPU6050 FOUND");
    }

    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
}

void init_bmp()
{
    debugln("BMP180 INITIALIZATION");
    if (!bmp.begin())
    {
        debugln("Could not find a valid BMP085 sensor, check wiring!");
        while (1)
        {
            // TODO: add beep to notify
        }
    }
    else
    {
        debugln("BMP180 FOUND");
    }
}

// function to initialize bmp, mpu, lora module and the sd card module
void init_sensors()
{
 
    init_bmp();
    init_mpu();
}

// Get the sensor readings
struct SensorReadings get_readings()
{
    struct SensorReadings return_val;
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    return_val.altitude = bmp.readAltitude(SEA_LEVEL_PRESSURE);
    return_val.ax = a.acceleration.x;
    return_val.ay = a.acceleration.y;
    return_val.az = a.acceleration.z;
    return_val.temperature=bmp.readTemperature();

    return_val.gx = g.gyro.x;
    return_val.gy = g.gyro.y;
    return_val.gz = g.gyro.z;

    return return_val;
}

#endif