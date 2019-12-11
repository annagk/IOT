#include <ESP8266WiFi.h>
#include <PubSubClient.h>

int measurePin = A0;
int ledPower = 4;
char str_voMeasured[10];
char str_calcVoltage[10];
char str_dustDensity[10];

unsigned int samplingTime = 280;
unsigned int deltaTime = 40;
unsigned int sleepTime = 9680;

float voMeasured = 0;
float voMeasured_old = 0;
float calcVoltage = 0;
float dustDensity = 0;
float dustDensity_old = 0;

// Update these with values suitable for your network.
// my hotspot
const char* ssid = "AGK";
const char* password = "91AuiaV00";
//const char* mqtt_server = "broker.mqtt-dashboard.com";
const char* mqtt_server = "172.20.10.4";




WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[100];
int value = 0;


void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  //Serial.begin(9600);
  pinMode(ledPower,OUTPUT);
}



void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("TopicDustReads", "");
      // ... and resubscribe
      client.subscribe("outTopicRed");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}





void loop(){
  
 if (!client.connected()) {
    reconnect();
   }
  client.loop(); 

  
  digitalWrite(ledPower,LOW);
  delayMicroseconds(samplingTime);

  dustDensity_old = dustDensity;
  delay(15000);
  voMeasured = analogRead(measurePin);
  delayMicroseconds(deltaTime);
  digitalWrite(ledPower,HIGH);
  delayMicroseconds(sleepTime);


  // 0 - 3.3V mapped to 0 - 1023 integer values
  // recover voltage
  calcVoltage = voMeasured * (3.3/ 1024.0);
 
  // linear eqaution taken from http://www.howmuchsnow.com/arduino/airquality/
  // Chris Nafis (c) 2012
  dustDensity = 0.17 * calcVoltage - 0.1;
 
  Serial.print("Raw Signal Value (0-1023): ");
  Serial.print(voMeasured);
 
  Serial.print(" - Voltage: ");
  Serial.print(calcVoltage);

 
  Serial.print(" - Dust Density: ");
  Serial.println(dustDensity);
 
 


dtostrf(voMeasured, 4, 2, str_voMeasured);
dtostrf(calcVoltage, 4, 2, str_calcVoltage);
dtostrf(dustDensity, 4, 2, str_dustDensity);

 sprintf(msg,"{\"RawMeasured\": %s,\"CalcVolt\": %s,\"dustDen\": %s }",str_voMeasured,str_calcVoltage,str_dustDensity);
 
    //sprintf(msg, "test");

    if (dustDensity != dustDensity_old) {
      
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish("TopicDustReads", msg);

    }
}
