#ifndef TRANSMITLORA_H
#define TRANSMITLORA_H

#include "heltec.h"
#include <SPI.h>
#include "defs.h"
#include "functions.h"
#include "readsensors.h"

char *printTransmitMessageLoRa(SendValues sv)
{
//the assigned size is calculated to fit the string
 char *message = (char *)pvPortMalloc(60);
 
 if (!message)
 return NULL;
 
 snprintf(message, 60, "{\"timestamp\":%lld,\"state\":%d,\"altitude\":%.3f}\n", sv.timeStamp, sv.state, sv.altitude);
 return message;
}

void sendTelemetryLora(SendValues sv[5])
{
LoRa.beginPacket();

/*
* LoRa.setTxPower(txPower,RFOUT_pin);
* txPower -- 0 ~ 20
* RFOUT_pin could be RF_PACONFIG_PASELECT_PABOOST or RF_PACONFIG_PASELECT_RFO
*   - RF_PACONFIG_PASELECT_PABOOST -- LoRa single output via PABOOST, maximum output 20dBm
*   - RF_PACONFIG_PASELECT_RFO     -- LoRa single output via RFO_HF / RFO_LF, maximum output 14dBm
*/
  LoRa.setTxPower(14,RF_PACONFIG_PASELECT_PABOOST);

char combinedMessage[300];
strcpy(combinedMessage, "");
for (int i=0; i<5 ; i++){
char *message= printTransmitMessageLoRa(sv[i]);
strcat(combinedMessage, message);
vPortFree(message);
}
LoRa.print(combinedMessage);
LoRa.endPacket();
}

void handleLora(SendValues sv[5])
{
sendTelemetryLora(sv);
}

#endif