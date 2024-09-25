#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Replace these with your network credentials
const char *ssid = "Kauane";
const char *password = "27030476";

WiFiClient client;

String macAddress;

void setup()
{
    macAddress = WiFi.macAddress();

    Serial.begin(115200);

    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED);

    // Send HTTP request after WiFi connection
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;

        // Specify the URL and pass the WiFiClient object
        String url = "http://192.168.0.104:3000/api/esp/" + macAddress;
        http.begin(client, url); // Replace with your server URL

        // Send HTTP GET request
        int httpCode = http.GET();

        // Check the returning code
        if (httpCode > 0)
        {
            Serial.printf("GET... code: %d\n", httpCode);

            // File found at server
            if (httpCode == HTTP_CODE_OK)
            {
                String payload = http.getString();
                Serial.println("Response:");
                Serial.println(payload);
            }
        }
        else
        {
            Serial.printf("GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end(); // Free resources
    }
}

void loop()
{
    Serial.println(macAddress);
}
