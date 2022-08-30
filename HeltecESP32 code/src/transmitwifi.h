#ifndef TRANSMITWIFI_H
#define TRANSMITWIFI_H

#include <WiFi.h>
#include "defs.h"
#include "functions.h"



// void create_Accesspoint()
// {
// debugln();
// debug("creating access point ")
// debugln("ssid: ")
// debugln(ssid);
// debugln("password: ")
// debugln(password);
//  WiFi.softAP(ssid, password);
//   IPAddress IP = WiFi.softAPIP();
//   debugln("Access point successully created ");
//   debugln("IP address: ")
//   debugln(IP);

//   client.setServer(mqtt_server, MQQT_PORT);

// }
void setup_wifi()
{
  // Connect to a WiFi network
  debugln();
  debug("Connecting to ");
  debugln(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    debug(".");
  }

  debugln("");
  debugln("WiFi connected");
  debugln("IP address: ");
  debugln(WiFi.localIP());

  client.setServer(mqtt_server, MQQT_PORT);
  //client.setCallback(mqttCallback);
}
void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    debugln("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client"))
    {
      debugln("connected");
    }
    else
    {
      debug("failed, rc=");
      debug(client.state());
      debugln(" try again in 50 milliseconds");
      // Wait 5 seconds before retrying
      delay(50);
    }
  }
}


//use this to send GPS coordinates
void sendGPS(SendValues sv[5])
{

  for (int i = 0; i < 5; i++)
  {
    // publish whole message i json
    char mqttMessage[200];
    sprintf(mqttMessage, "{\"longitude\":%.8f,\"latitude\":%.8f}",sv[i].longitude, sv[i].latitude);
    client.publish("esp32/GPS", mqttMessage);
  }
}


void handleGPS(SendValues sv[5])
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  sendGPS(sv);
}



#endif