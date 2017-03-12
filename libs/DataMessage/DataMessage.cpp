#include "DataMessage.h"
#include "Arduino.h"

const unsigned long dataMessageSize = sizeof(dataMessage);

char * serializeMessage(dataMessage sData){
    char pBuffer[dataMessageSize];
    
    memcpy(pBuffer, &sData, dataMessageSize);
    return pBuffer;
    
}

dataMessage deserialize(char *serData){
    dataMessage packageData;
    memcpy(&packageData, serData, dataMessageSize);
    return packageData;
}


uint16_t calcCHKSum(dataMessage msg){
    uint16_t checkSum = msg.sensData.zoneId +
    msg.sensData.temperature +
    msg.sensData.hummidity +
    dataMessageSize;
    
    return checkSum;
}

