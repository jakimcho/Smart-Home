/*
 * This node is responsible to collect data from DHT and MQ2 sensors and to send it vie serial to MQTT publisher node (ESP6288)
 * MQTT publisher node is placed in DataPublisher scheme
 */

#include "DHT.h"
#include "DataMessage.h"

#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

#define DHT_ZONE1 2 // DHT22 seonsor 1 read in pin
#define DHT_ZONE2 3 // DHT22 seonsor 2 read in pin
#define DHT_ZONE3 4 // DHT22 seonsor 3 read in pin

#define MQ_ZONE1 A2 // MQ2 seonsor 1 read in pin
#define MQ_ZONE2 A3 // MQ2 seonsor 2 read in pin
#define MQ_ZONE3 A4 // MQ2 seonsor 3 read in pin

#define ZONE1 0
#define ZONE2 1
#define ZONE3 2

int mqs[] = {MQ_ZONE1, MQ_ZONE2, MQ_ZONE3};
int zones[] = {ZONE1, ZONE2, ZONE3};
DHT dhts[3] = {DHT(DHT_ZONE1, DHTTYPE), DHT(DHT_ZONE2, DHTTYPE), DHT(DHT_ZONE3, DHTTYPE)};


char * ZONE_HEADERS[] = {"Floor 1", "Floor 2", "Floor 3"};

void setup() {
  Serial.begin(115200);
  Serial.println("\n Start System");
    for(int i = 0; i < 3; i++){
      dhts[i].begin();
    }
}

void loop() {
  Serial.println("\nStart getting data");
  
  unsigned short checkSum;
  unsigned long uBufSize = sizeof(dataMessage);
  char pBuffer[uBufSize];
  uint16_t temperature, hummidity, mq2;

  for(int i = 0; i < 3; i++){
    Serial.print("\nReading data for zone "); Serial.println(zones[i]);
    getDHTData(dhts[i], &temperature, &hummidity);
    Serial.print("\nReading MQ number "); Serial.println(mqs[i]);
    mq2 = getMQ2Data(mqs[i]);
    
    dataMessage packageData = prepareDataMessage(zones[i], temperature, hummidity, mq2);
    Serial.print("\nSize of the structure is: "); Serial.println(uBufSize);
    
    Serial.print("Data header: "); Serial.println(packageData.header);
    Serial.print("Data Check Sum: "); Serial.println(packageData.checkSum);
    Serial.print("Temperature: "); Serial.println(packageData.sensData.temperature);
    Serial.print("Hummidity: "); Serial.println(packageData.sensData.hummidity);
    Serial.print("MQ2: "); Serial.println(packageData.sensData.mq2);
    Serial.print("Zone Id: "); Serial.println(packageData.sensData.zoneId);
    
    memcpy(pBuffer, &packageData, uBufSize);
    free(pBuffer);
    delay(1000);
  }

  delay(5000);
}

/*
 * Getting data from specific sensor
 * param:
 *  - sensorId specify the sensor to get data from
 * 
 * return: sensor data
 */
uint16_t getMQ2Data(int sensorId){
  uint16_t gasLevel = analogRead(sensorId);
  return gasLevel;
}

/*
 * Getting data from specific sensor
 * param:
 *  - sensorId specify the sensor to get data from
 *  - temperature used to return the temperature data multyplyed by 1000 (to bypass the need of float)
 *  - hummidity used to return the hummidity data multyplyed by 1000 (to bypass the need of float)
 * 
 * return: 0 on success
 */
int getDHTData(DHT dht, uint16_t *temperature, uint16_t *hummidity){
  Serial.print("\n ----------------- Enter getDHTData --------------");
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("\nProblem while raeading DHT data:");
    return -1;
  }

  Serial.print("\nTemperature and Hummdity before multiplication: ");
  Serial.print(t);Serial.print(" and ");Serial.println(h);
  *temperature = (uint16_t)(t * 100);
  *hummidity = (uint16_t)(h * 100);
  
  Serial.print("\nTemperature and Hummdity after multiplication: ");
  Serial.print(*temperature);Serial.print(" and ");Serial.println(*hummidity);

  Serial.print("\n ----------------- Exiting getDHTData --------------");
  return 0;
}

/*
 * Initialize dataMessage structure
 * params:
 *  - sensorId specify the sensor to get data from
 *  - temperature used to return the temperature data multyplyed by 1000 (to bypass the need of float)
 *  - hummidity used to return the hummidity data multyplyed by 1000 (to bypass the need of float)
 * 
 * return: 0 on success
 */
dataMessage prepareDataMessage(int zone, uint16_t temperature, uint16_t hummidity, uint16_t mq2){
  dataMessage tmpDM;
  sensorsData tmpSD;
  
  tmpSD.temperature = temperature;
  tmpSD.hummidity = hummidity;
  tmpSD.mq2 = mq2;
  tmpSD.zoneId = zone;
  strcpy(tmpDM.header, ZONE_HEADERS[zone]);
  tmpDM.sensData = tmpSD;
  tmpDM.checkSum = calcCHKSum(tmpDM);
  
  return tmpDM;
}




