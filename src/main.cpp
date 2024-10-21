// --------------------------- Libs ---------------------------
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <string>
#include <WiFiClient.h>


// ------------------ Functions declarations ------------------
void connectToWiFi();
void registerEsp();
bool verifyTag(const char* hash);

// --------------------- Global Constants ---------------------
bool printed = false;

// --------------------- Global Variables ---------------------
String ssid = "Murylão";
String password = "playsdomurylo";
String token = "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE3Mjk0Nzc0NzAsInN1YiI6IjE4MmFiZDNlLWE3YTktNDNkYi1hYmQ5LTg3YzUzMDkyODczOSJ9.uD5XCfF-Fvh9YUebnVF17_WeVZf2qMPZ2FAovsecMUM";
String espToken = "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJwZXREb29ySWQiOiIxZWI4Mzc5YS04NTk5LTRkNTQtYmZhZC05YWZkMTdiNDZjMzMiLCJ1c2VySWQiOiIxODJhYmQzZS1hN2E5LTQzZGItYWJkOS04N2M1MzA5Mjg3MzkiLCJpYXQiOjE3Mjk1MTczOTR9.21bJt-a9yrZlSXiax5jB4FTffqWd2UL-Zj5KrRzKKBk";


// --------------------------- Setup --------------------------
void setup()
{
    Serial.begin(115200);
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

    verifyTag("hash1");
    verifyTag("hash2");
    verifyTag("hash3");
    verifyTag("hash4");
    Serial.println();
    delay(2000);
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
        espToken = http.getString();
    }
    else
    {
        Serial.println(response);
        Serial.println(http.getString());
    }

    http.end();
}

bool verifyTag(const char* hash) {
    HTTPClient http;
    WiFiClient wifiClient;

    char endpoint[100];
    strcpy(endpoint, "http://192.168.164.137:3000/api/esp/pet/");
    strcat(endpoint, hash);
    http.begin(wifiClient, endpoint);

    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", espToken);

    int response = http.GET();
    http.end();

    Serial.printf("\n%s authorization: %d", hash, response);
    return response == 204;
}
