/*
 * HelTec Automation(TM) Low_Power test code, witch includ
 * Function summary:
 *
 * - Vext connected to 3.3V via a MOS-FET, the gate pin connected to GPIO21;
 *
 * - OLED display and PE4259(RF switch) use Vext as power supply;
 *
 * - WIFI Kit series V1 don't have Vext control function;
 * 
 * - Basic LoRa Function;
 *
 * - Esp_Deep_Sleep Function;
*/

#include "heltec.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60        /* Time ESP32 will go to sleep (in seconds) */

#define BAND    915E6  //you can set band here directly,e.g. 868E6,915E6
#define DHTPIN 13
#define DHTTYPE DHT11
DHT_Unified dht(DHTPIN, DHTTYPE);
// RTC_DATA_ATTR int bootCount = 0;
uint32_t delayMS;
char t[12], h[12];
float temp, humid;
/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
void send_lora(String message){
  LoRa.beginPacket();
  LoRa.setTxPower(20,RF_PACONFIG_PASELECT_PABOOST);
  LoRa.print(message);
  LoRa.endPacket();
}

void read_dht(){
  String tosend="";
  bool sendit=true;
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    // sendit=false;
  }
  else {
    temp = event.temperature;
    Serial.print("Temperature: ");
    Serial.print(temp);
    tosend.concat("{\"temperature\": ");
    tosend.concat(temp);
    tosend.concat(",");
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    // sendit=false;
    tosend.concat("}");
  }
  else {
    humid = event.relative_humidity;
    Serial.print("\tHumidity: ");
    Serial.print(humid);
    tosend.concat("\"humidity\": ");
    tosend.concat(humid);
    tosend.concat("}");
  }
  if(sendit){
    send_lora(tosend);
  }
  
  Serial.println();

  delay(delayMS);
}

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_GPIO:
    {
      Serial.println("Someone is in!");
      send_lora("{\"person\": 1}");
      delay(1000);
      while(digitalRead(12)==1){
              delay(1000);
      }
      Serial.println("Gone.");
      send_lora("{\"person\": 0}");
      delay(2);
    } break;
    case ESP_SLEEP_WAKEUP_TIMER:
    {
      Serial.println("Wakeup caused by TIMER");
      delay(2);
    } break;
    default :
    {
      Serial.println("Wakeup was not caused by deep sleep");
      delay(2);
    } break;
  }
}

void setup(){
  //WIFI Kit series V1 not support Vext control
  Heltec.begin(false /*DisplayEnable Enable*/, true /*LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
  gpio_wakeup_enable(GPIO_NUM_12, GPIO_INTR_HIGH_LEVEL);
  esp_sleep_enable_gpio_wakeup();
  delay(10);

  pinMode(12,INPUT);
  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  delayMS = sensor.min_delay / 1000;
  delay(100);
}

void loop(){
  //This is not going to be called
  read_dht();
  Serial.println("Going to sleep now");
  delay(20);
  esp_light_sleep_start();
  print_wakeup_reason();
}