# fauxmoESP

Belkin WeMo emulator library for ESP8266.

This is a library for ESP8266-based devices that emulates a Belkin WeMo device and thus allows you to control them using this protocol, in particular from Alexa-powered devices like the Amazon Echo or the Dot.

This library is a port of Maker Musings' [Fauxmo Python library][6].

## Dependencies

This library uses [ESPAsyncTCP][3] and [ESPAsyncUDP][4] libraries by [me-no-dev][5].

If you are using PlatformIO (check the section bellow on how to compile it) you can install them by adding the dependencies to your ```platformio.ini``` file:

```
lib_install = 305,359
```

Otherwise you will have to manually install them from sources.

## Usage

The library is very easy to use, basically instantiate an object, connect to the Wifi, add one or more virtual devices and bind the callback to get the messages. An schematic example could be:

```
#include <fauxmoESP.h>

fauxmoESP fauxmo;

void setup() {

    Serial.begin(115200);

    ... connect to wifi ...

    fauxmo.addDevice("light one");
    fauxmo.addDevice("light two");
    fauxmo.onMessage([](const char * device_name, bool state) {
        Serial.printf("%s state: %s\n", device_name, state ? "ON" : "OFF");
    });

}

void loop() {

}

```

(Check the examples folder for a working example.)

Then run the "discover devices" option from your Alexa app or web (in the Smart Home section). A new device with the name you have configured should appear. Tell Alexa to switch it on or off and check your terminal ;)

## Compiling with PlatformIO

The library uses ```listenMulticast``` method from AsyncUDP to join the multicast group where the controllers send broadcast messages to identify compatible devices. This method relies on ```udp_set_multicast_netif_addr``` which requires the latest git version of the [Arduino Core for ESP8266][1] (after Jul. 11, 2016).

At the moment, PlatformIO is using the stable version so there is no support for it. To enable the staging version of the espressif8266 platform you should follow [this steps][2] to install the development version. Basically you have to run:

```
pio platform install https://github.com/platformio/platform-espressif8266.git#feature/stage
```

The ```platformio.ini``` file in the examples is already configured to use the staging version.

## Compiling with Arduino IDE

Same applies to the Arduino IDE. You will need to use the development version of the ESP8266 Arduino Core. Steps to use the library are:

* Install the [latest ESP8266 Arduino Core using these instructions](https://github.com/esp8266/Arduino#using-git-version) (remove before the stable version from your Boards Manager if any).
* Copy or checkout the ESPAsyncTCP and ESPAsyncUDP libraries in your arduino/libraries folder, it should be under “My Documents/Arduino/libraries” in Windows or “Documents/Arduino/libraries” in Mac or Linux unless you have placed it somewhere else.
* Same for the fauxmoESP library, check it out in the arduino/libraries folder.
* Restart your Arduino IDE
* Look for the fauxmoESP_Basic example under File > Examples > fauxmoESP > …
* Choose your board and compile.


[1]:https://github.com/esp8266/Arduino
[2]:http://docs.platformio.org/en/stable/platforms/espressif8266.html#using-arduino-framework-with-staging-version
[3]:https://github.com/me-no-dev/ESPAsyncTCP
[4]:https://github.com/me-no-dev/ESPAsyncUDP
[5]:https://github.com/me-no-dev
[6]:https://github.com/makermusings/fauxmo
