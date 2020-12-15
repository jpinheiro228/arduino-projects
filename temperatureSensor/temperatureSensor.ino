// QoS levels:
// 0 - Publisher sends the message and no ACK is needed.
// 1 - Publisher sends message ans wait for ACK from broker. If no ACK, Resend Package.
// 2 - Publisher sends message, Broker ACK, Pub ACK and Broker ACK.

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define DHTPIN  25
#define DHTTYPE DHT11
#define RESET_PIN 33

const char *ID = "bb-node02";  // Name of our device, must be unique
const char *TOPIC = "house/room/temperature";  // Topic to publish to
const char *TOPIC2 = "house/room/humidity";  // Topic to publish to

const char* ssid     = "The_Babies2.4";
const char* password = "pipocacomnutella";

IPAddress broker(192,168,0,5); // IP address of your MQTT broker eg. 192.168.1.50
WiFiClient wclient;

PubSubClient client(wclient); // Setup MQTT client

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;
float temp=0, humid=0;
char i[10],j[10];

void reconnect() {
  // Reconnect to client
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(ID)) {
      Serial.println("connected");
      Serial.println('\n');

    } else {
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, HIGH);

  Serial.begin(115200);
  // Initialize device.
  dht.begin();
  WiFi.begin(ssid, password);
  int count = 0;

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      count++;
        if(count==20){
          digitalWrite(RESET_PIN, LOW);
        }
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(broker, 1883);

  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  delayMS = sensor.min_delay / 1000;
}

void loop() {

  if (!client.connected()){  // Reconnect if connection is lost
    reconnect();
  }
  client.loop();
  
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    temp = event.temperature;
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    humid = event.relative_humidity;
  }
  
  sprintf(i,"%.2f\t",temp);
  sprintf(j,"%.2f",humid);
  client.publish(TOPIC, i);
  client.publish(TOPIC2, j);
  delay(delayMS);
}
