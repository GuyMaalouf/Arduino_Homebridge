/*  The Workhop - Arduino Homebridge Control V1.0 - February 2021
 *   
 *  This sketch will allow you to control a LED 
 *  and read the temperature and humidity on your apple devices 
 *  by using homebridge and the Apple homekit app.
 *  Homebridge is an opensource platform that allows you to create a server
 *  on you Windoes, Mac, Linux, Docker or Raspberry pi devices
 *  
 *  Homebridge creates a link between the Apple Home Accessory Protocol 
 *  and the protocol of your smart devices such as HTTP and MQTT
 *  
 *  The needed libraries for the DHT11 can be downloaded here: 
 *  - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
 *  - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor
 *  See guide for details on sensor wiring and usage: 
 *  https://learn.adafruit.com/dht/overview
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

const char* ssid = "SSID";
const char* password = "Password";

#define LEDPIN 12 // Pin D6 on the NodeMCU 12-E
#define DHTPIN  2 // Pin D4 on the NodeMCU 12-E
#define DHTTYPE DHT11

// Create an instance of the server
// specify the port to listen on as an argument
ESP8266WebServer server(80);

DHT dht(DHTPIN, DHTTYPE);

float humidity, temp_c;  // Values read from sensor

void handle_root() {
  server.send(200, "text/plain", "Homebridge http Temperature and humidity Sensor, read from /dht");
  delay(100);
}

// Generally, you should use "unsigned long" for variables that hold time
unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 500;              // interval at which to read sensor in ms

void setup() {
  
  Serial.begin(115200);
  delay(10);
  pinMode(LEDPIN, OUTPUT);     // Initialize the LEDPIN pin as an output (Very important on NodeMCU!)
  
  dht.begin();    //Initialize the DHT sensor
  
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  delay(500);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Handeling different HTTP requests sent from the homebridge server
  server.on("/", handle_root);  // Root handeling

  server.on("/dht", []() {      // Temperature and Humidity Request
    gettemperature();
    Serial.println("Sending");
    server.send(200, "application/json", "{\n" "\"temperature\": " + String(temp_c) + ",\n ""\"humidity\": " + String(humidity) + "" "\n}");
    Serial.println("Sent!");
  });

  server.on("/LedOn", []() {    // Request to turn the LED on
    digitalWrite(LEDPIN,HIGH);
    Serial.println("LED On");
    server.send(200, "text/html", "LED is ON!");
    Serial.println("Sent!");
  });

  server.on("/LedOff", []() {    // Request to turn the LED off
    digitalWrite(LEDPIN,LOW);
    Serial.println("LED Off");
    server.send(200, "text/html", "LED is OFF!");
    Serial.println("Sent!");
  });
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {
  server.handleClient();
}

void gettemperature() {
  Serial.println("Who wants to know the temp?");
  // if the difference between the current time and last time you read
  // the sensor is bigger than the interval you set, read the sensor
  // Works better than delay for things happening elsewhere also
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you read the sensor
    previousMillis = currentMillis;

    // Reading temperature for humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
    humidity = dht.readHumidity();          // Read humidity (percent)
    temp_c = dht.readTemperature();     // Read temperature as Fahrenheit
    Serial.print("Temp: ");Serial.print(temp_c);Serial.print(" . -  Hum: ");Serial.print(humidity);
    // Check if any reads failed and exit early (to try again).
    if (isnan(humidity) || isnan(temp_c)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
  }
}
