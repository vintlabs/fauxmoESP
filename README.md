# FauxmoESP

Amazon Alexa support for ESP8266 and ESP32 devices.

This is a library for ESP8266/ESP32-based devices that emulates a Belkin WeMo device and thus allows you to control them using this protocol, in particular from Alexa-powered devices like the Amazon Echo or the Dot.

## Origin

* This library is a port of Maker Musings' [Fauxmo Python library][6].
* ESP8266 Code by Xose Pérez <xose dot perez at gmail dot com>
* Additional ESP32 Code by Frank Hellmann <frank at vfx dot to>
* Support for v2 devices by Bibi Blocksberg

**Current Version is 2.4.2**, this version shows some backwards incompatibilities with version 1.0.0. Check the examples to rewrite your code if you were using a previous version and read the [changelog](CHANGELOG.md).

**Note about Gen2 devices**, the preliminary support in version 2.4.0 has not been tested in depth (not by me at least). Original author (Bibi Blocksberg) has reported it is working only when compiled against **Arduino Core 2.3.0 for ESP8266**. Latest stable version of Arduino Core for ESP8266 is 2.4.0 and it has been reported as NOT WORKING with second generation devices like the Echo Plus. Also, it has not been tested with ESP32 devices. Please read issue #39 and linked issues before reporting new info about the subject.

## Dependencies

ESP8266:

* This library uses [ESPAsyncTCP][3] library by [me-no-dev][5]

ESP32:

* This library uses [AsyncTCP][4] library by [me-no-dev][5]

### PlatformIO

If you are using PlatformIO (check the section bellow on how to compile it) you can install them by adding the dependencies to your ```platformio.ini``` file:

```
lib_deps =
    ESPAsyncTCP
    AsyncTCP
```

### Arduino IDE

You will need to install the required library from sources. Your best option is to download the library as a ZIP file and install it using the option under "Sketch > Include Library > Add .ZIP Library...".

You can look for it manually but I have gathered the URL here for convenience:

|Device|Library|Repository|ZIP|
|-|-|-|-|
|ESP8266|**ESPAsyncTCP** by Hristo Gochkov ESP8266|[GIT](https://github.com/me-no-dev/ESPAsyncTCP)|[ZIP](https://github.com/me-no-dev/ESPAsyncTCP/archive/master.zip)|
|ESP32|**AsyncTCP** by Hristo Gochkov ESP32|[GIT](https://github.com/me-no-dev/AsyncTCP)|[ZIP](https://github.com/me-no-dev/AsyncTCP/archive/master.zip)|

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
    fauxmo.addDevice("light three");
    fauxmo.addDevice("light four");
    fauxmo.enable(true);

    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state) {
        Serial.printf("[MAIN] Device #%d (%s) state: %s\n", device_id, device_name, state ? "ON" : "OFF");
    });
    fauxmo.onGetState([](unsigned char device_id, const char * device_name) {
        return true; // whatever the state of the device is
    });

}

void loop() {
    fauxmo.handle();
}

```

(Check the examples folder)

Then run the "discover devices" option from your Alexa app or web (in the Smart Home section). A new device with the name you have configured should appear. Tell Alexa to switch it on or off and check your terminal ;)

## Device discovery

Device discovery can be incomplete when you have lots of devices defined. Since version 2.0.0 different strategies are used to maximize the chance of getting all of them discovered during the first round.

Tests have been run with up to 16 devices with success but your experience might be different. If not all of them are discovered on the first run, execute the Discover Devices option again from your Alexa app or tell your Echo/Dot to do it. Once they pop up in your devices list (even if they are flagged as "Offline") they should work just fine.

The strategies the library uses to improve discoverability are:

* Space UDP responses to help Echo/Dot and the device itself to perform setup queries
* Repeat UDP responses for devices not queried
* Randomize UDP responses

[1]:https://github.com/esp8266/Arduino
[2]:http://docs.platformio.org/en/stable/platforms/espressif8266.html#using-arduino-framework-with-staging-version
[3]:https://github.com/me-no-dev/ESPAsyncTCP
[4]:https://github.com/me-no-dev/AsyncTCP
[5]:https://github.com/me-no-dev
[6]:https://github.com/makermusings/fauxmo
