/*

FAUXMO ESP 2.0.0

Copyright (C) 2016 by Xose Pérez <xose dot perez at gmail dot com>

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

*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "fauxmoESP.h"

void fauxmoESP::_sendUDPResponse(unsigned int device_id) {

    fauxmoesp_device_t device = _devices[device_id];
    DEBUG_MSG_FAUXMO("[FAUXMO] UDP response for device #%d (%s)\n", _current, device.name);

    char buffer[16];
    IPAddress ip = WiFi.localIP();
    sprintf(buffer, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

    char response[strlen(UDP_TEMPLATE) + 40];
    sprintf_P(response, UDP_TEMPLATE,
        buffer,
        _base_port + _current,
        device.uuid,
        device.uuid
    );

    _udp.beginPacket(_remoteIP, _remotePort);
    _udp.write(response);
    _udp.endPacket();

}

void fauxmoESP::_nextUDPResponse() {

    while (_roundsLeft) {
        if (_devices[_current].hit == false) break;
        if (++_current == _devices.size()) {
            --_roundsLeft;
            _current = 0;
        }
    }

    if (_roundsLeft > 0) {
        _sendUDPResponse(_current);
        if (++_current == _devices.size()) {
            --_roundsLeft;
            _current = 0;
        }
    }
}

void fauxmoESP::_handleUDPPacket(IPAddress remoteIP, unsigned int remotePort, uint8_t *data, size_t len) {

    if (!_enabled) return;

    data[len] = 0;

    if (strstr((char *) data, UDP_SEARCH_PATTERN) == (char *) data) {
        if (strstr((char *) data, UDP_DEVICE_PATTERN) != NULL) {

            #ifdef DEBUG_FAUXMO
                char buffer[16];
                sprintf(buffer, "%d.%d.%d.%d", remoteIP[0], remoteIP[1], remoteIP[2], remoteIP[3]);
                DEBUG_MSG_FAUXMO("[FAUXMO] Search request from %s\n", buffer);
            #endif

            // Set hits to false
            for (unsigned int i = 0; i < _devices.size(); i++) {
                _devices[i].hit = false;
            }

            // Send responses
            _remoteIP = remoteIP;
            _remotePort = remotePort;
            _current = random(0, _devices.size());
            _roundsLeft = UDP_RESPONSES_TRIES;

        }
    }

}

void fauxmoESP::_handleSetup(AsyncWebServerRequest *request, unsigned int device_id) {

    if (!_enabled) return;

    DEBUG_MSG_FAUXMO("[FAUXMO] Device #%d /setup.xml\n", device_id);
    _devices[device_id].hit = true;

    fauxmoesp_device_t device = _devices[device_id];
    char response[strlen(SETUP_TEMPLATE) + 50];
    sprintf_P(response, SETUP_TEMPLATE, device.name, device.uuid);
    request->send(200, "text/xml", response);

}

void fauxmoESP::_handleContent(AsyncWebServerRequest *request, unsigned int device_id, char * content) {

    if (!_enabled) return;

    DEBUG_MSG_FAUXMO("[FAUXMO] Device #%d /upnp/control/basicevent1\n", device_id);
    fauxmoesp_device_t device = _devices[device_id];

    if (strstr(content, "<BinaryState>0</BinaryState>") != NULL) {
        if (_callback) _callback(device_id, device.name, false);
    }

    if (strstr(content, "<BinaryState>1</BinaryState>") != NULL) {
        if (_callback) _callback(device_id, device.name, true);
    }

}

void fauxmoESP::addDevice(const char * device_name) {

    fauxmoesp_device_t new_device;
    unsigned int device_id = _devices.size();

    // Copy name
    new_device.name = strdup(device_name);

    // Create UUID
    char uuid[15];
    sprintf(uuid, "444556%06X%02X\0", ESP.getChipId(), device_id); // "DEV" + CHIPID + DEV_ID
    new_device.uuid = strdup(uuid);

    // TCP Server
    new_device.server = new AsyncWebServer(_base_port + device_id);
    new_device.server->on("/setup.xml", HTTP_GET, [this, device_id](AsyncWebServerRequest *request){
        _handleSetup(request, device_id);
    });
    new_device.server->on("/upnp/control/basicevent1", HTTP_POST,
        [](AsyncWebServerRequest *request) {
            request->send(200);
        },
        NULL,
        [this, device_id](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            data[len] = 0;
            _handleContent(request, device_id, (char *) data);
        }
    );

    new_device.server->begin();

    // Attach
    _devices.push_back(new_device);

    DEBUG_MSG_FAUXMO("[FAUXMO] Device '%s' added (#%d)\n", device_name, device_id);

}

void fauxmoESP::handle() {

    int len = _udp.parsePacket();
    if (len > 0) {
        IPAddress remoteIP = _udp.remoteIP();
        unsigned int remotePort = _udp.remotePort();
        uint8_t data[len];
        _udp.read(data, len);
        _handleUDPPacket(remoteIP, remotePort, data, len);
    }

    if (_roundsLeft > 0) {
        if (millis() - _lastTick > UDP_RESPONSES_INTERVAL) {
            _lastTick = millis();
            _nextUDPResponse();
        }
    }

}

void fauxmoESP::enable(bool enable) {
    DEBUG_MSG_FAUXMO("[FAUXMO] %s\n", enable ? "Enabled" : "Disabled");
    _enabled = enable;
}

fauxmoESP::fauxmoESP(unsigned int port) {

    _base_port = port;

    // UDP Server
    _udp.beginMulticast(WiFi.localIP(), UDP_MULTICAST_IP, UDP_MULTICAST_PORT);

}
