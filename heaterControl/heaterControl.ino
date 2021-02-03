// Heater Control using MQTT protocol.
// QoS levels:
// 0 - Publisher sends the message and no ACK is needed.
// 1 - Publisher sends message ans wait for ACK from broker. If no ACK, Resend Package.
// 2 - Publisher sends message, Broker ACK, Pub ACK and Broker ACK.

#include <WiFi.h>
#include <PubSubClient.h>

#define RELAY_PIN 32

const char *ID = "bb-node01";  // Name of our device, must be unique
const char *PUBTOPIC = "house/heater";  // Topic to publish to
const char *SUBTOPIC = "house/heater/set"; // Topic to subscribe to
const char *SUBTOPIC2 = "house/heater/get"; // Topic to subscribe to

const char* ssid     = "The_Babies2.4";
const char* password = "pipocacomnutella";

IPAddress broker(192,168,0,5); // IP address of your MQTT broker eg. 192.168.1.50
WiFiClient wclient;

PubSubClient client(wclient); // Setup MQTT client

char i[10],j[10];
int flag=0;

void callback(char* topic, byte* payload, unsigned int length) {
  String myPayload="";
  for (int i=0;i<length;i++) {
    myPayload+=(char)payload[i];
  }
  if (strcmp(topic,SUBTOPIC)==0){
    if (myPayload=="ON"){digitalWrite(RELAY_PIN, LOW);PubStatus();}
    else if (myPayload=="OFF"){digitalWrite(RELAY_PIN, HIGH);PubStatus();}
    
  }
  else if (strcmp(topic,SUBTOPIC2)==0){
    PubStatus();
  }
}
void PubStatus(){
  Serial.println("Publishing Heater Status");
  if (!digitalRead(RELAY_PIN)) {client.publish(PUBTOPIC, "ON", true);Serial.println("ON");}
  else {client.publish(PUBTOPIC, "OFF", true);Serial.println("OFF");}
}

void reconnect() {
  // Reconnect to client
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(ID)) {
      Serial.println("connected");
      Serial.println('\n');

      // Subscribe to your topics here!
      client.subscribe(SUBTOPIC);
      client.subscribe(SUBTOPIC2);
      delay(2000);
      client.publish(SUBTOPIC2, "STATUS", true);
    } else {
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
    wifi_connect();
  }
}

void wifi_connect(){
  if (WiFi.status() != WL_CONNECTED){
    Serial.println("WIFI is not connected. Attempting to reconnect...");
    int count = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        count++;
          if(count==20){
            ESP.restart();
          }
    }
  }
}

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  Serial.begin(115200);
  // Initialize device.
  WiFi.setHostname(ID);
  delay(1000);
  WiFi.begin(ssid, password);
  int count = 0;

  wifi_connect();
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Setting up MQTT Breker
  client.setServer(broker, 1883);
  client.setCallback(callback);
}

void loop() {
  
  wifi_connect();
  
  if (!client.connected()){  // Reconnect if connection is lost
    reconnect();
  }
  
  client.loop();
}
