# fauxmoESP

Amazon Alexa support for ESP8266 devices.

This is a library for ESP8266-based devices that emulates a Belkin WeMo device and thus allows you to control them using this protocol, in particular from Alexa-powered devices like the Amazon Echo or the Dot.

This library is a port of Maker Musings' [Fauxmo Python library][6].

**Current Version is 2.0.0**, this version shows some backwards incompatibilities with version 1.0.0. Check the examples to

## Dependencies

This library uses [ESPAsyncTCP][3], [ESPAsyncWebServer][7] and (optionally) [ESPAsyncUDP][4] libraries by [me-no-dev][5].

### PlatformIO

If you are using PlatformIO (check the section bellow on how to compile it) you can install them by adding the dependencies to your ```platformio.ini``` file:

```

lib_deps = ESPAsyncTCP ESPAsyncWebServer
```

### Arduino IDE

You will need to install the required libraries from sources. Your best option is to download the library as a ZIP file and install it using the option under "Sketch > Include Library > Add .ZIP Library...".

You can look for them manually but I have gathered the URLs to those ZIP files here for convenience:

|Library|Repository|ZIP|Notes|
|-|-|-|-|
|**ESPAsyncTCP** by Hristo Gochkov|[GIT](https://github.com/me-no-dev/ESPAsyncTCP)|[ZIP](https://github.com/me-no-dev/ESPAsyncTCP/archive/master.zip)||
|**ESPAsyncWebServer** by Hristo Gochkov|[GIT](https://github.com/me-no-dev/ESPAsyncWebServer)|[ZIP](https://github.com/me-no-dev/ESPAsyncWebServer/archive/master.zip)||
|**ESPAsyncUDP** by Hristo Gochkov|[GIT](https://github.com/me-no-dev/ESPAsyncUDP)|[ZIP](https://github.com/me-no-dev/ESPAsyncUDP/archive/master.zip)|Only if using "async" mode, see below|

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
    fauxmo.onMessage([](unsigned char device_id, const char * device_name, bool state) {
        Serial.printf("[MAIN] Device #%d (%s) state: %s\n", device_id, device_name, state ? "ON" : "OFF");
    });

}

void loop() {
    fauxmoESP.handle();
}

```

(Check the examples folder)

Then run the "discover devices" option from your Alexa app or web (in the Smart Home section). A new device with the name you have configured should appear. Tell Alexa to switch it on or off and check your terminal ;)

## Using the "Async" version

Since fauxmoESP 2.0 the library uses the "compatibility" mode by default, this means that it uses WiFiUdp class instead of AsyncUDP.
The later requires the Arduino Core for ESP8266 staging version whilst the former works fine with current stable 2.3.0 version.
But, since it's not "async" anymore we have to manually poll for UDP packets. If you want to forget about polling you can compile it against the AsyncUDP library
by passing "COMPATIBLE_2_3_0=0" flag when building (or adding it to your sketch).

While we wait for an updated version of the Arduino Core for ESP8266 you will need to use the staging version of it to use the "async" mode. Here you can find the
instructions to set up your environment:

### Async mode with PlatformIO

The library uses ```listenMulticast``` method from AsyncUDP to join the multicast group where the controllers send broadcast messages to identify compatible devices. This method relies on ```udp_set_multicast_netif_addr``` which requires the latest git version of the [Arduino Core for ESP8266][1] (after Jul. 11, 2016).

At the moment, PlatformIO is using the stable version so there is no support for it. To enable the staging version of the espressif8266 platform you should follow [this steps][2] to install the development version. Basically you have to run:

```

pio platform install https://github.com/platformio/platform-espressif8266.git#feature/stage
```

The ```platformio.ini``` file in the examples is already configured to use the staging version.

Also: remember to **add ESPAsyncUDP** library to your lib_deps key in platformio.ini.

### Async mode with Arduino IDE

Same applies to the Arduino IDE. You will need to use the development version of the ESP8266 Arduino Core. Steps to use the library are:

* Install the [latest ESP8266 Arduino Core using these instructions](https://github.com/esp8266/Arduino#using-git-version) (remove before the stable version from your Boards Manager if any).
* Copy or checkout the ESPAsyncUDP library (see ZIP above) in your arduino/libraries folder, it should be under “My Documents/Arduino/libraries” in Windows or “Documents/Arduino/libraries” in Mac or Linux unless you have placed it somewhere else.
* Same for the fauxmoESP library, check it out in the arduino/libraries folder.
* Restart your Arduino IDE
* Look for the fauxmoESP_Async example under File > Examples > fauxmoESP > …
* Choose your board and compile.


[1]:https://github.com/esp8266/Arduino
[2]:http://docs.platformio.org/en/stable/platforms/espressif8266.html#using-arduino-framework-with-staging-version
[3]:https://github.com/me-no-dev/ESPAsyncTCP
[4]:https://github.com/me-no-dev/ESPAsyncUDP
[5]:https://github.com/me-no-dev
[6]:https://github.com/makermusings/fauxmo
[7]:https://github.com/me-no-dev/ESPAsyncWebServer
