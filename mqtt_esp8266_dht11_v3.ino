/*
 Basic ESP8266 MQTT example

 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.

 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.

 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

*/
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// Update these with values suitable for your network.
#define DHTPIN D4     // what pin we're connected to
#define DHTTYPE DHT11   // DHT11 or DHT22 (AM2302) or DHT21 (AM2301)

const char* ssid = "2tiwehorklj";
const char* password = "BBbdpttptptmktt@2302";
//const char* mqtt_server = "broker.mqtt-dashboard.com";
const char* mqtt_server = "test.mosquitto.org";

Adafruit_BMP085 bmp;
DHT dht(DHTPIN, DHTTYPE);
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
      //client.publish("jobox/temp1", "hello world");
      // ... and resubscribe
      client.subscribe("jobox/temp1");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  dht.begin();
  bmp.begin();
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int p = bmp.readPressure();
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(2000);
    return;
  }
  Serial.print("{\"Humidity\": ");
  Serial.print(h);
  Serial.print(" %");
  Serial.print(", \"Temperature\": ");
  Serial.print(t);
  Serial.print(" c");
  Serial.print(", \"Pressure\": ");
  Serial.print(p);
  Serial.println(" Pa}");

  delay(2000);
  
  
  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    //msg=outline;
    snprintf (msg, 100, "{\"humidity\":%.1f,\"temperature\": %.1f, \"pressure\": %.d}",h,t,p);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("jobox/temp1", msg);
  }
}
