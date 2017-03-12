#ifndef DataMessage.h
#define DataMessage.h

#include "Arduino.h"

typedef struct __attribute((__packed__)) SENSORS_DATA //__attribute((__packed__)) it is very important, else stupid ESP calculate size strangely
{
  uint8_t zoneId;
  uint16_t mq2;
  uint16_t temperature;
  uint16_t hummidity;
} sensorsData;

typedef struct __attribute((__packed__)) DATA_PACKAGE{
  char header[16];
  sensorsData sensData;
  long checkSum;
} dataMessage;

char * serializeMessage(dataMessage sData);
dataMessage deserialize(char *serData);
uint16_t calcCHKSum(dataMessage msg);

#endif
