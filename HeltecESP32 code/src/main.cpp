#include <Arduino.h>
#include "heltec.h"
#include <checkState.h>
#include <logdata.h>
#include <readsensors.h>
#include <transmitwifi.h>
#include <transmitlora.h>
#include <defs.h>
#include <kalmanfilter.h>

TimerHandle_t ejectionTimerHandle = NULL;

portMUX_TYPE mutex = portMUX_INITIALIZER_UNLOCKED;

TaskHandle_t loraTelemetryTaskHandle;

TaskHandle_t GetDataTaskHandle;

TaskHandle_t SDWriteTaskHandle;

TaskHandle_t GPSTaskHandle;



float BASE_ALTITUDE = 0;

float previousAltitude;

volatile int state = 0;

static uint16_t lora_queue_length = 100;
static uint16_t sd_queue_length = 500;
static uint16_t gps_queue_length = 100;

static QueueHandle_t lora_telemetry_queue;
static QueueHandle_t sdwrite_queue;
static QueueHandle_t gps_queue;

struct Data readData(){
  Data dt ={0};
  SensorReadings readings ={0};
  FilteredValues filtered_v = {0};

  readings = get_readings();
  // change the value of readings.ay based on the orientation of the gyroscope
  filtered_v = kalmanUpdate(readings.altitude,readings.ay);

  //using mutex to modify the state
    portENTER_CRITICAL(&mutex);
    state = checkState(filtered_v.displacement, previousAltitude, filtered_v.velocity, filtered_v.acceleration, state);
    portEXIT_CRITICAL(&mutex);
    previousAltitude = filtered_v.displacement;

  dt = formart_data(readings, filtered_v);
  dt.state = state;
  dt.timeStamp = millis();

  return dt;
}

/*
**********Time Taken for each Task******************
        Get Data Task  - 36ms
        LORa task  - 392ms
        WiFi task  - 74ms
        GPS Task - 1000ms
        SD Write Task - 60ms
*/

void GetDataTask(void *parameter)
{

    struct Data dt = {0};
    struct SendValues sv = {0};

    static int droppedLoRaPackets = 0;
    static int droppedSDPackets = 0;

    for (;;)
    {

        dt = readData();
        sv = formart_send_data(dt);

        if (xQueueSend(lora_telemetry_queue, (void *)&sv, 0) != pdTRUE)
        {
            droppedLoRaPackets++;
        }
        if (xQueueSend(sdwrite_queue, (void *)&dt, 0) != pdTRUE)
        {
            droppedSDPackets++;
        }

        debugf("Dropped LoRa Packets : %d\n", droppedLoRaPackets);
        debugf("Dropped SD Packets : %d\n", droppedSDPackets);

        // yield to SD write task
        vTaskDelay(960 / portTICK_PERIOD_MS);
    }
}

void readGPSTask(void *parameter)
{

    struct GPSReadings gpsReadings = {0};

     static int droppedGPSPackets = 0;

    for (;;)
    {
        gpsReadings = get_gps_readings();
       

        if (xQueueSend(gps_queue, (void *)&gpsReadings, 0) != pdTRUE)
        {
            droppedGPSPackets++;
        }

        debugf("Dropped GPS Packets : %d\n", droppedGPSPackets);

        // yield LORA and WiFi task
        // TODO: increase this up from 60 to 1000 in steps of 60 to improve queue performance at the expense of GPS
        //GPS will send 1 reading in 2s when set to 1000
        vTaskDelay(466 / portTICK_PERIOD_MS);
    }
}

void loraTelemetryTask(void *parameter)
{
    struct SendValues sv = {0};
    struct SendValues svRecords[5];
    struct GPSReadings gpsReadings = {0};
    float latitude = 0;
    float longitude = 0;

    for (;;)
    {

        for (int i = 0; i < 5; i++)
        {
            xQueueReceive(lora_telemetry_queue, (void *)&sv, 10);
            svRecords[i] = sv;
            svRecords[i].latitude = latitude;
            svRecords[i].longitude = longitude;

            if (xQueueReceive(gps_queue, (void *)&gpsReadings, 10) == pdTRUE)
            {
                latitude = gpsReadings.latitude;
                longitude = gpsReadings.longitude;
            }
        }
        
        handleLora(svRecords);
        handleGPS(svRecords);
        // yield to Gps task
        vTaskDelay(1000/ portTICK_PERIOD_MS);
    }
}

void SDWriteTask(void *parameter)
{

    struct Data dt = {0};
    struct Data dtRecords[5];
    struct GPSReadings gps = {0};
    float latitude = 0;
    float longitude = 0;

    for (;;)
    {

        for (int i = 0; i < 5; i++)
        {
            xQueueReceive(sdwrite_queue, (void *)&dt, 10);

            dtRecords[i] = dt;
            dtRecords[i].latitude = latitude;
            dtRecords[i].longitude = longitude;

            if (xQueueReceive(gps_queue, (void *)&gps, 10) == pdTRUE)
            {
                latitude = gps.latitude;
                longitude = gps.longitude;
            }
        }
        appendToFile(dtRecords);

        // yield to Getdata Task
        vTaskDelay(36 / portTICK_PERIOD_MS);
    }
}

void setup() {

    //initialize the heltec
    Heltec.begin(true /*DisplayEnable Enable*/, true /*LoRa Disable*/, true /*Serial Enable*/, true /*LoRa use PABOOST*/, BAND /*LoRa RF working band*/);
    Serial.begin(BAUD_RATE);
    //create_Accesspoint();
      setup_wifi();
    init_sensors();

    initSDCard();
     
    // get the base_altitude
    BASE_ALTITUDE = get_base_altitude();

    lora_telemetry_queue = xQueueCreate(lora_queue_length, sizeof(SendValues));
    sdwrite_queue = xQueueCreate(sd_queue_length, sizeof(Data));
    gps_queue = xQueueCreate(gps_queue_length, sizeof(GPSReadings));

    // initialize core tasks
    xTaskCreatePinnedToCore(GetDataTask, "GetDataTask", 3000, NULL, 1, &GetDataTaskHandle, 0);
    xTaskCreatePinnedToCore(loraTelemetryTask, "loraTelemetryTask", 4000, NULL, 1, &loraTelemetryTaskHandle, 1);
   xTaskCreatePinnedToCore(readGPSTask, "ReadGPSTask", 3000, NULL, 1, &GPSTaskHandle, 1);
    xTaskCreatePinnedToCore(SDWriteTask, "SDWriteTask", 4000, NULL, 1, &SDWriteTaskHandle, 0);

}

void loop() {
  // put your main code here, to run repeatedly:
}