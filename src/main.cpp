// --------------------------- Libs ---------------------------
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <string>
#include <WiFiClient.h>
#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WebServer.h>
#include <WiFiClientSecure.h>
#include <EEPROM.h>


// ------------------ Functions declarations ------------------
void connectToWiFi(String ssid, String password);
void registerEsp(String userToken);
bool verifyTag(String UID);
String getTagUID(MFRC522 mfrc522);
void handleRoot();
void handleLogin();
String getUserToken(String email, String password);
void startAP();

// --------------------- Global Constants ---------------------
#define SS_PIN D2
#define RST_PIN D1
#define SERVO_PIN D4

// --------------------- Global Variables ---------------------
String espToken = "";

Servo doorLock;
MFRC522 mfrc522(SS_PIN, RST_PIN);

ESP8266WebServer server(80);
const char* apSSID = "DeviceSetup";

// --------------------------- Setup --------------------------
void setup()
{
    Serial.begin(115200);

    SPI.begin();
    mfrc522.PCD_Init();

    doorLock.attach(SERVO_PIN); 
    doorLock.write(0);

    startAP();
}


// --------------------------- Loop ---------------------------
void loop()
{
    // await for user to setup through AP Mode
    if(espToken.length() == 0)
    {
        server.handleClient();
        return;
    }

    // Look for new RFID cards+
    if (!mfrc522.PICC_IsNewCardPresent())
        return;
    // Select one of the RFID tags
    if (!mfrc522.PICC_ReadCardSerial())
        return;
    
    String tagUID = getTagUID(mfrc522);
    Serial.println(tagUID);

    if(verifyTag(tagUID))
    {
        doorLock.write(90);
        delay(5000);
        doorLock.write(0);
    }
    else
    {
        delay(1000);
    }
}

void connectToWiFi(String ssid, String password) 
{
    WiFi.mode(WIFI_STA);
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

void registerEsp(String userToken) 
{
    HTTPClient http;
    WiFiClient wifiClient;
    http.begin(wifiClient, "http://192.168.0.105:3000/api/esp");

    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", userToken);

    int response = http.POST("");
    if(response == 201) 
    {
        String body = http.getString();
        Serial.println(body);
        String res = body.substring(1, body.length() - 1);
        espToken = "Bearer " + res;
    }

    http.end();
}

bool verifyTag(String UID) 
{
    HTTPClient http;
    WiFiClient wifiClient;

    String endpoint = "http://192.168.0.105:3000/api/esp/pet/" + UID;
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

String getTagUID(MFRC522 mfrc522) 
{
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

void startAP() 
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSSID);
    IPAddress IP = WiFi.softAPIP();
    Serial.println("AP mode started");
    Serial.print("Server IP address: ");
    Serial.println(IP);

    server.on("/", handleRoot);
    server.on("/login", handleLogin);
    server.begin();
}

void handleRoot() 
{
    String htmlPage = "<form action='/login' method='POST'>"
                      "WiFi SSID: <input name='ssid'><br>"
                      "WiFi Password: <input name='wifiPassword' type='password'><br>"
                      "Email: <input name='email'><br>"
                      "Password: <input name='password' type='password'><br>"
                      "<input type='submit' value='Submit'>"
                      "</form>";
    server.send(200, "text/html", htmlPage);
}

void handleLogin() 
{
    String form_ssid = server.arg("ssid");
    String form_wifi_password = server.arg("wifiPassword");
    String form_email = server.arg("email");
    String form_password = server.arg("password");

    connectToWiFi(form_ssid, form_wifi_password);
    String userToken = getUserToken(form_email, form_password);
    registerEsp(userToken);
}

String getUserToken(String email, String password)
{
    HTTPClient http;
    WiFiClient wifiClient;

    String endpoint = "http://192.168.0.105:3000/api/login";
    String payload = "{\"email\":\"" + email + "\",\"password\":\"" + password + "\"}";
    http.begin(wifiClient, endpoint);
    http.addHeader("Content-Type", "application/json");

    int responseCode = http.POST(payload);
    String responseBody = http.getString();

    Serial.println(responseCode);
    Serial.println(responseBody);

    int tokenStart = responseBody.indexOf("token\":\"") + 8;
    int tokenEnd = responseBody.indexOf("\"", tokenStart);
    http.end();

    return "Bearer " + responseBody.substring(tokenStart, tokenEnd);
} 
