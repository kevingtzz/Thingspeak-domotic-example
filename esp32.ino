#include <WiFi.h>
#include <Servo.h>
#include <DHT.h>
#include "ThingSpeak.h"
#include "secrets.h"

#define wifiled 2
#define lamp 19
#define fan 18
#define DHTPIN 15
#define DHTTYPE DHT11
#define switch_humidity 12
#define switch_temperature 26

int keyIndex = 0;            // your network key Index number (needed only for WEP)
int lamp_field = 1;
int fan_field = 2;                //First field in ThingSpeak
int door_field = 3;
int humidity_field = 4;
int temp_field = 5;
int light_field = 6;
int ldr_pin = 35;
float coeficiente_porcentaje=100.0/4096.0; // it must be 4095 'cuase esp32 have 12bits pins
long lamp_status = 0;
long fan_status = 0;
long door_status = 20;
unsigned long previousMillis = 0;        // will store last time
const long interval = 60000;           // interval to write light
WiFiClient wifi_client;
Servo door;
DHT dht(DHTPIN, DHTTYPE);

void setup() {
    Serial.begin(115200);         // Start the Serial communication to send messages to the computer
    ThingSpeak.begin(wifi_client);  // Initialize ThingSpeak
    dht.begin();
    delay(10);
    Serial.println('\n');

    pinMode(wifiled, OUTPUT);            //define pinMode
    pinMode(lamp, OUTPUT);
    pinMode(fan, OUTPUT);
    pinMode(switch_humidity, INPUT);
    pinMode(switch_temperature, INPUT);

    door.attach(13);

    connect_to_Wifi(WIFI_SSID, WIFI_PASSWORD);    // Connect to the network, replace the WIFI_SSID and WIFI_PASSWORD in secrets.h

    lamp_status = read_lamp();
    fan_status = read_fan();
    door_status = read_door();

}

void loop() { 
    unsigned long currentMillis = millis();

    long temp_lamp_status = read_lamp();
    if (lamp_status != temp_lamp_status) {
        lamp_status = temp_lamp_status;
        Serial.println("Reading light...");
        Serial.printf("light status: %d \n", lamp_status);
    }

    long temp_fan_status = read_fan();
    if (fan_status != temp_fan_status) {
        fan_status = temp_fan_status;
        Serial.println("Reading fan...");
        Serial.printf("fan status: %d \n", fan_status);
    }

    long temp_door_status = read_door();
    if (door_status != temp_door_status) {
        door_status = temp_door_status;
        Serial.println("Reading door...");
        Serial.printf("door status: %d \n", door_status);
    }

    if (digitalRead(switch_humidity)) {
        write_humidity();
        delay(1000);
    }

    if (digitalRead(switch_temperature)) {
        write_temp();
        delay(1000);
    }

    if (currentMillis - previousMillis >= interval) {
        write_light();
        previousMillis = currentMillis;
    }
}

void connect_to_Wifi(const char* ssid, const char* password) {
    WiFi.begin(ssid, password);             
    Serial.printf("Connecting to %s", ssid);

    while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
        delay(500);
        Serial.print('.');
    }

    Serial.println('\n');
    Serial.println("Connection established!");  
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer
    digitalWrite(wifiled, HIGH);
}

long read_lamp() {
    long lamp_status = ThingSpeak.readLongField(CHANNEL_ID, lamp_field, READ_APIKEY);
    // Check the status of the read operation to see if it was successful
    int statusCode = ThingSpeak.getLastReadStatus();
    if (statusCode != 200) {
        Serial.println("Problem reading channel. HTTP error code " + String(statusCode));
    }
    digitalWrite(lamp, lamp_status);
    return lamp_status;
}

long read_fan() {
    long fan_status = ThingSpeak.readLongField(CHANNEL_ID, fan_field, READ_APIKEY);
    // Check the status of the read operation to see if it was successful
    int statusCode = ThingSpeak.getLastReadStatus();
    if (statusCode != 200) {
        Serial.println("Problem reading channel. HTTP error code " + String(statusCode));
    }
    digitalWrite(fan, fan_status);
    return fan_status;
}

long read_door() {
    long door_status = ThingSpeak.readLongField(CHANNEL_ID, door_field, READ_APIKEY);
    // Check the status of the read operation to see if it was successful
    int statusCode = ThingSpeak.getLastReadStatus();
    if (statusCode != 200) {
        Serial.println("Problem reading channel. HTTP error code " + String(statusCode));
    }
    door.write(door_status);    
    return fan_status;
}

void write_humidity() {
    float humidity = dht.readHumidity();
    Serial.print("Humidity: ");
    Serial.println(humidity);
    int status = ThingSpeak.writeField(CHANNEL_ID, humidity_field, int(humidity), WRITE_APIKEY);
    if (status != 200) {
        Serial.println("Problem updating channel. HTTP error code " + String(status));
    }
}

void write_temp() {
    float temp = dht.readTemperature();
    Serial.print("Temperature: ");
    Serial.println(temp);
    int status = ThingSpeak.writeField(CHANNEL_ID, temp_field, int(temp), WRITE_APIKEY);
    if (status != 200) {
        Serial.println("Problem updating channel. HTTP error code " + String(status));
    }
}

void write_light() {
    int light_value = analogRead(ldr_pin)*coeficiente_porcentaje;
    Serial.printf("Light: %d", light_value);
    int status = ThingSpeak.writeField(CHANNEL_ID, light_field, light_value, WRITE_APIKEY);
    if (status != 200) {
        Serial.println("Problem updating channel. HTTP error code " + String(status));
    }
}