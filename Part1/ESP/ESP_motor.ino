#include <pwmWrite.h>
#include <WiFi.h>
#include <PubSubClient.h>
#define PIN_SG90 27 // Broche de sortie utilisée

// Replace the next variables with your SSID/Password combination
const char* ssid = "CPE-Domotique";
const char* password = "astro4student";

// Add your MQTT Broker IP address, example:
const char* mqtt_server = "192.168.1.113";

WiFiClient espClient;
PubSubClient client(espClient);

Pwm pwm = Pwm();

void setup() {
  Serial.begin(9600);

  setup_wifi();
  client.setServer(mqtt_server, 1884);
  client.setCallback(callback);
}

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

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ServoClient")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("/I102/CoudraisGirardin/air_flow");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "/I102/CoudraisGirardin/air_flow") {
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    String messageTemp;
    
    for (int i = 0; i < length; i++) {
      Serial.print((char)message[i]);
      messageTemp += (char)message[i];
    }
    Serial.print("\n");

    //If airflow is 0 we close the shutter
    if(messageTemp == "0"){
      pwm.writeServo(PIN_SG90, 0);
    }
    //If airflow is 1 we open the shutter
    if(messageTemp == "1"){
      pwm.writeServo(PIN_SG90, 170);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}