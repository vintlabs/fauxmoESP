# FauxmoESP

Amazon Alexa support for ESP8266 and ESP32 devices.

This is a library for ESP8266/ESP32-based devices that emulates Philips Hue lights and thus allows you to control them using this protocol, in particular from Alexa-powered devices like the Amazon Echo or the Dot.

[![version](https://img.shields.io/badge/version-3.0.2-brightgreen.svg)](CHANGELOG.md)
[![license](https://img.shields.io/badge/license-MIT-orange.svg)](LICENSE)
[![donate](https://img.shields.io/badge/donate-PayPal-blue.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=xose%2eperez%40gmail%2ecom&lc=US&no_note=0&currency_code=EUR&bn=PP%2dDonationsBF%3abtn_donate_LG%2egif%3aNonHostedGuest)
[![twitter](https://img.shields.io/twitter/follow/xoseperez.svg?style=social)](https://twitter.com/intent/follow?screen_name=xoseperez)

## History

Before version 3.0.0, the library used a different protocol (emulating Belkin Wemo devices). The library was a port of Maker Musings' [Fauxmo Python library][6] to the ESP8266 platform. Support for ESP32 and Gen2 devices was added by Frank Hellmann <frank at vfx dot to> and Bibi Blocksberg respectively.

Since version 3.0.0 the library uses a different approach and emulates Philips Hue lights instead. This allows for a simpler code and also support for numeric values (you can now say "Alexa, set light to 50"). This version has been inspired by the [node-red-contrib-alexa-local](https://github.com/originallyus/node-red-contrib-alexa-local) plugin by originallyus for NodeRED and the [ESPalexa](https://github.com/Aircoookie/Espalexa) library by Christian Schwinne.

## Dependencies

Besides the libraries already included with the Arduino Core for ESP8266 or ESP32, these libraries are also required to use fauxmoESP:

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

    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
        Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
    });

}

void loop() {
    fauxmo.handle();
}

```

(Check the examples folder)

Then run the "discover devices" option from your Alexa app or web (in the Smart Home section). A new device with the name you have configured should appear. Tell Alexa to switch it on or off and check your terminal ;)

## Troubleshooting

The `onGetState` method accepts a function (a callback) that will be called when a new message arrives. Try not to do many things inside the callback, it should return as fast as possible. Instead of adding logic here just save the data (device_id and state, for instance) and process it from your main loop.

[1]:https://github.com/esp8266/Arduino
[2]:http://docs.platformio.org/en/stable/platforms/espressif8266.html#using-arduino-framework-with-staging-version
[3]:https://github.com/me-no-dev/ESPAsyncTCP
[4]:https://github.com/me-no-dev/AsyncTCP
[5]:https://github.com/me-no-dev
[6]:https://github.com/makermusings/fauxmo

## License

Copyright (C) 2016-2018 by Xose Pérez <xose dot perez at gmail dot com>

The MIT License (MIT)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
