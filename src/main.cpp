// --------------------------- Libs ---------------------------
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <string>
#include <WiFiClient.h>
#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>


// ------------------ Functions declarations ------------------
void connectToWiFi();
void registerEsp();
bool verifyTag(String UID);
String getTagUID(MFRC522 mfrc522);

// --------------------- Global Constants ---------------------
#define SS_PIN D2
#define RST_PIN D1
#define SERVO_PIN D4

// --------------------- Global Variables ---------------------
String ssid = "Murylão";
String password = "playsdomurylo";

String token = "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE3Mjk0Nzc0NzAsInN1YiI6IjE4MmFiZDNlLWE3YTktNDNkYi1hYmQ5LTg3YzUzMDkyODczOSJ9.uD5XCfF-Fvh9YUebnVF17_WeVZf2qMPZ2FAovsecMUM";
String espToken = "";

Servo doorLock;
MFRC522 mfrc522(SS_PIN, RST_PIN);


// --------------------------- Setup --------------------------
void setup()
{
    Serial.begin(115200);

    SPI.begin();
    mfrc522.PCD_Init();

    doorLock.attach(SERVO_PIN); 
    doorLock.write(0);
}


// --------------------------- Loop ---------------------------
void loop()
{
    // If user has already sent token with wifi info
    if(token.length() > 0) 
    {
        // Connect to WiFi
        if(WiFi.status() != WL_CONNECTED) 
            connectToWiFi();

        // Send door register request in case you don't have the token already
        if(espToken.length() == 0)
        {
            registerEsp();
            Serial.print("Token: ");
            Serial.println(espToken);
        }
    }

    // Look for new RFID cards+
    if (!mfrc522.PICC_IsNewCardPresent())
        return;
    // Select one of the RFID tags
    if (!mfrc522.PICC_ReadCardSerial())
        return;
    
    String tagUID = getTagUID(mfrc522);

    if(verifyTag(tagUID))
    {
        delay(2000);
        doorLock.write(90);
        delay(2000);
        doorLock.write(0);
    }
}

void connectToWiFi() {
    WiFi.begin(ssid, password);
    Serial.print("\nConnecting to Wi-Fi");

    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        Serial.print(".");
    }

    Serial.print("\nWiFi connected. IP address: ");
    Serial.println(WiFi.localIP());
}

void registerEsp() {
    HTTPClient http;
    WiFiClient wifiClient;
    http.begin(wifiClient, "http://192.168.164.137:3000/api/esp");

    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", token);

    int response = http.POST("");
    if(response == 201) 
    {
        String body = http.getString();
        String res = body.substring(1, body.length() - 1);
        espToken = "Bearer " + res;
    }

    http.end();
}

bool verifyTag(String UID) {
    HTTPClient http;
    WiFiClient wifiClient;

    String endpoint = "http://192.168.164.137:3000/api/esp/pet/" + UID;
    http.begin(wifiClient, endpoint);

    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", espToken);

    int response = http.GET();
    http.end();

    Serial.println();
    Serial.print(UID);
    Serial.printf(" authorization: %d", response);
    return response == 204;
}

String getTagUID(MFRC522 mfrc522) {
    String uidString = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        if (mfrc522.uid.uidByte[i] < 0x10) {
            uidString += "0";
        }
        uidString += String(mfrc522.uid.uidByte[i], HEX);
    }
    uidString.toUpperCase();
    return uidString;
}