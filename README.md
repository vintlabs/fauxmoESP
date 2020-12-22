# FauxmoESP

Amazon Alexa support for ESP8266 and ESP32 devices.

This is a library for ESP8266/ESP32-based devices that emulates Philips Hue lights and thus allows you to control them using this protocol, in particular from Alexa-powered devices like the Amazon Echo or the Dot.

[![version](https://img.shields.io/badge/version-3.1.2-brightgreen.svg)](CHANGELOG.md)
[![codacy](https://img.shields.io/codacy/grade/44478ddd58fe4cc6a2bc5598232663b8/master.svg)](https://www.codacy.com/app/xoseperez/fauxmoesp/dashboard)
[![license](https://img.shields.io/badge/license-MIT-orange.svg)](LICENSE)

[![donate](https://img.shields.io/badge/donate-PayPal-blue.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=xose%2eperez%40gmail%2ecom&lc=US&no_note=0&currency_code=EUR&bn=PP%2dDonationsBF%3abtn_donate_LG%2egif%3aNonHostedGuest)
[![twitter](https://img.shields.io/twitter/follow/xoseperez.svg?style=social)](https://twitter.com/intent/follow?screen_name=xoseperez)

## NEW! Discussions on Github
We now have a place for general discussion, ideas, projects etc. - https://github.com/vintlabs/fauxmoESP/discussions/133

## Notice

**As of October 2020 https://github.com/vintlabs/fauxmoESP is the new home for fauxmoESP!!**

**Many thanks for all of the work that Xose Perez has put into this project!**

*I have migrated all of the issues from the old repo on Bitbucket here to GitHub, and I have closed many stale issues. If I have closed an issue that you feel should still be open, feel free to reopen it or submit a new one.*


## History

2020-12-22 Version 3.2 released. Devices now show properly in the Alexa App as a bulb.

2020-12-08 Version 3.1.2 released. New version available in Arduino Library Manager or at https://github.com/vintlabs/fauxmoESP/releases/tag/3.1.2  PlatformIO is still pending update.

Before version 3.0.0, the library used a different protocol (emulating Belkin Wemo devices). The library was a port of Maker Musings' [Fauxmo Python library][6] to the ESP8266 platform. Support for ESP32 and Gen2 devices was added by Frank Hellmann <frank at vfx dot to> and Bibi Blocksberg respectively.

Since version 3.0.0 the library uses a different approach and emulates Philips Hue lights instead. This allows for a simpler code and also support for numeric values (you can now say "Alexa, set light to 50"). This version has been inspired by the [node-red-contrib-alexa-local](https://github.com/originallyus/node-red-contrib-alexa-local) plugin for NodeRED and the [ESPalexa](https://github.com/Aircoookie/Espalexa) library by Christian Schwinne.

## Dependencies

Besides the libraries already included with the Arduino Core for ESP8266 or ESP32, these libraries are also required to use fauxmoESP:

ESP8266:

* This library uses [ESPAsyncTCP][3] library by [me-no-dev][5]

ESP32:

* This library uses [AsyncTCP][4] library by [me-no-dev][5]

### PlatformIO

If you are using PlatformIO (check the section bellow on how to compile it) the required libraries should be installed automatically.

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

    fauxmo.setPort(80); // required for gen3 devices
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

## To use with ESP-IDF

Add `#include "Arduino.h"`


Then run the "discover devices" option from your Alexa app or web (in the Smart Home section). A new device with the name you have configured should appear. Tell Alexa to switch it on or off and check your terminal ;)

## Troubleshooting

Current status of the library:

|Platform|Gen1|Gen2|Gen3|
|---|---|---|---|
|ESP8266 Core 2.3.X|OK|OK?|OK (1)|
|ESP8266 Core 2.4.0|OK (2)|OK? (2)|OK (1, 2)|
|ESP8266 Core 2.4.1|OK (2)|OK? (2)|OK (1, 2)|
|ESP8266 Core 2.4.2|OK (2)|OK? (2)|OK (1, 2)|
|ESP32|OK|OK?|OK (1)|

(1) When using gen3 devices TCP port must be 80 always.
(2) Arduino Core for ESP8266 requires LwIP set to "v1.4 Higher Bandwidth".

* fauxmoESP 3.1.X: When using with gen3 devices TCP port must be 80. You can define it with the `setPort` method.

* fauxmoESP 3.1.X: If you application already uses port 80 you can prevent fauxmoESP from creating its own webserver and inject the values from your application handlers to the library. Check the fauxmoESP_External_Server example.

* fauxmoESP 3.X.X: When using Arduino Core for ESP8266 v2.4.X, double check you are building the project with LwIP Variant set to "v1.4 Higher Bandwidth". You can change it from the Tools menu in the Arduino IDE or passing the `-DPIO_FRAMEWORK_ARDUINO_LWIP_HIGHER_BANDWIDTH` build flag to PlatformIO.

* fauxmoESP 2.X.X: The `onGetState` method accepts a function (a callback) that will be called when a new message arrives. Try not to do many things inside the callback, it should return as fast as possible. Instead of adding logic here just save the data (device_id and state, for instance) and process it from your main loop.

* Some people have reported problems when the ESP and the Alexa devices are connected to different wireless networks (like 2.4 and 5GHz bands on some routers). See https://bitbucket.org/xoseperez/fauxmoesp/issues/53.

* Latest version of ESP Async Webserver fails building with the current setup. Use version 1.2.2 max. See fauxmoESP_External_Server example `platformio.ini` file.

[1]:https://github.com/esp8266/Arduino
[2]:http://docs.platformio.org/en/stable/platforms/espressif8266.html#using-arduino-framework-with-staging-version
[3]:https://github.com/me-no-dev/ESPAsyncTCP
[4]:https://github.com/me-no-dev/AsyncTCP
[5]:https://github.com/me-no-dev
[6]:https://github.com/makermusings/fauxmo

## If you enjoy this, please consider supporting us by purchasing a module from us!
http://www.vintlabs.com

## License

Copyright (C) 2016-2020 by Xose Pérez <xose dot perez at gmail dot com>, 2020 by Paul Vint <pjvint at gmail dot com>

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
