#include <Arduino.h>
#ifdef ESP32
    #include <WiFi.h>
#else
    #include <ESP8266WiFi.h>
#endif

#include "fauxmoESP.h"

// Rename the credentials.sample.h file to credentials.h and 
// edit it according to your router configuration
#include "credentials.h"
// or alternately set SSID and passphrase below
//#define WIFI_SSID "----"
//#define WIFI_PASS "----"

fauxmoESP fauxmo;

#define SERIAL_BAUDRATE     115200

#define DEVICE_NAME "Baboon"

// Set RGB pins
#define REDPIN 21
#define GREENPIN 22
#define BLUEPIN 23
// LED channels
#define REDC 1
#define GREENC 2
#define BLUEC 3


// -----------------------------------------------------------------------------
// Wifi
// -----------------------------------------------------------------------------

void wifiSetup() {

    // Set WIFI module to STA mode
    WiFi.mode(WIFI_STA);

    // Connect
    Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    // Wait
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }
    Serial.println();

    // Connected!
    Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());

}



// HSV-RGB stuff 
uint8_t redLed;
uint8_t greenLed;
uint8_t blueLed;


void setup() 
{
    Serial.begin(SERIAL_BAUDRATE);

    // Wifi
    wifiSetup();

    fauxmo.setPort(80); // This is required for gen3 devices
    fauxmo.enable(true);
    fauxmo.addDevice(DEVICE_NAME);  

    // LED strip testing
    ledcAttachPin(REDPIN, REDC);
    ledcAttachPin(GREENPIN, GREENC);
    ledcAttachPin(BLUEPIN, BLUEC);
    ledcSetup(REDC, 12000, 8);  // 12kHz 8 bit
    ledcSetup(GREENC, 12000, 8);
    ledcSetup(BLUEC, 12000, 8);

    //fauxmo.setState((unsigned char) 0, (bool) 1, (unsigned char) 254);

    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value, unsigned char hue, unsigned int saturation, unsigned int ct) 
    {
      Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d hue: %u saturation: %u ct: %u\n", device_id, device_name, state ? "ON" : "OFF", value, hue, saturation, ct);

      char colormode[3];
      fauxmo.getColormode(device_id, colormode, 3);
      Serial.printf("Colormode: %s\n", colormode);
      
      redLed = fauxmo.getRed(device_id);
      greenLed = fauxmo.getGreen(device_id);
      blueLed = fauxmo.getBlue(device_id);
      
      Serial.printf("HSV: %d %d %d  RGB: %d %d %d\n", hue, saturation, value, redLed, greenLed, blueLed);
      
      if (state)
      {
        ledcWrite(REDC, redLed);
        ledcWrite(GREENC, greenLed);
        ledcWrite(BLUEC, blueLed);
      }
      else
      {
        
        ledcWrite(REDC, 0);
        ledcWrite(GREENC, 0);
        ledcWrite(BLUEC, 0);
      }
    });

    
}

void loop() {
    fauxmo.handle();
}
