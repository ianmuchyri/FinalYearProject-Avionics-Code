#include <Arduino.h>
#include <defs.h>
#include <checkState.h>
#include <readsensors.h>
#include <transmit.h>
#include <kalmanfilter.h>

TimerHandle_t ejectionTimerHandle = NULL;
portMUX_TYPE mutex = portMUX_INITIALIZER_UNLOCKED;
TaskHandle_t wifiTelemetryTaskHandle;
TaskHandle_t getDataTaskHandle;
//if 1 chute has been deployed
uint8_t isChuteDeployed = 0;
float BASE_ALTITUDE = 0;
float previousAltitude;
volatile int state = 0 ;
static uint16_t wifi_queue_length = 100;
static QueueHandle_t wifi_telemetry_queue;

// callback for done ejection
void ejectionTimerCallback(TimerHandle_t ejectionTimerHandle){
digitalWrite(EJECTION_PIN, LOW);
isChuteDeployed = 1;

}

// ejection fires the explosive charge using a relay or mosfet
void ejection(){
    if (isChuteDeployed == 0)
    {
        digitalWrite(EJECTION_PIN, HIGH);
        // TODO: is 3 seconds enough?
        ejectionTimerHandle = xTimerCreate("EjectionTimer", 3000 / portTICK_PERIOD_MS, pdFALSE, (void *)0, ejectionTimerCallback);
        xTimerStart(ejectionTimerHandle, portMAX_DELAY);
    }
}

Data readData(){
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

// this is based on data calculated from frequency measurements of individual tasks being carried out.
/*
**********Time Taken for each Task******************
        Get Data Task  - 36ms
        WiFiTelemetryTask -74ms

*/

void GetDataTask(void *parameter){
Data dt = {0};
//below is used to measure the number of dropped wifi packets
static int droppedWiFiPackets = 0;
 for (;;)
    {

        dt = readData();

        if (xQueueSend(wifi_telemetry_queue, (void *)&dt, 0) != pdTRUE)
        {
            droppedWiFiPackets++;
        }

        debugf("Dropped WiFi Packets : %d\n", droppedWiFiPackets);

    }
}

void WiFiTelemetryTask(void *parameter)
{
    struct Data dt = {0};

    for (;;)
    {

        xQueueReceive(wifi_telemetry_queue, (void *)&dt, 10);

        handleWiFi(dt);

    }
}

void setup() {

Serial.begin(BAUD_RATE);

//set up ejection pin
pinMode(EJECTION_PIN, OUTPUT);
//set up the wifi connection
setup_wifi();
// initialize the sensors
init_sensors();

//get base altitude
BASE_ALTITUDE = get_base_altitude();

wifi_telemetry_queue = xQueueCreate(wifi_queue_length, sizeof(Data));

//initialize core tasks
    xTaskCreatePinnedToCore(GetDataTask, "GetDataTask", 3000, NULL, 1, &getDataTaskHandle, 0);
    xTaskCreatePinnedToCore(WiFiTelemetryTask, "WiFiTelemetryTask", 4000, NULL, 1, &wifiTelemetryTaskHandle, 1);
}

void loop() {
  // put your main code here, to run repeatedly:
}