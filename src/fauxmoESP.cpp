/*

FAUXMO ESP 2.2.0

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

void fauxmoESP::_onUDPData(IPAddress remoteIP, unsigned int remotePort, void *data, size_t len) {

    if (!_enabled) return;

    char * p = (char *) data;
    p[len] = 0;

    if (strstr(p, UDP_SEARCH_PATTERN) == (char *) data) {
        if (strstr(p, UDP_DEVICE_PATTERN) != NULL) {

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

void fauxmoESP::_handleSetup(AsyncClient *client, unsigned int device_id) {

    DEBUG_MSG_FAUXMO("[FAUXMO] Device #%d /setup.xml\n", device_id);
    _devices[device_id].hit = true;
    fauxmoesp_device_t device = _devices[device_id];

    char response[strlen_P(SETUP_TEMPLATE) + 20];
    sprintf_P(response, SETUP_TEMPLATE, device.name, device.uuid);

    char headers[strlen_P(HEADERS) + 10];
    sprintf_P(headers, HEADERS, strlen(response));

    client->write(headers, strlen(headers));
    client->write(response, strlen(response));

}

void fauxmoESP::_handleContent(AsyncClient *client, unsigned int device_id, void *data, size_t len) {

    char content[len+1];
    memcpy(content, data, len);

    DEBUG_MSG_FAUXMO("[FAUXMO] Device #%d /upnp/control/basicevent1\n", device_id);
    fauxmoesp_device_t device = _devices[device_id];

    if (strstr(content, "<BinaryState>0</BinaryState>") != NULL) {
        if (_callback) _callback(device_id, device.name, false);
    }

    if (strstr(content, "<BinaryState>1</BinaryState>") != NULL) {
        if (_callback) _callback(device_id, device.name, true);
    }

    char headers[strlen_P(HEADERS) + 10];
    sprintf_P(headers, HEADERS, 0);
    client->write(headers, strlen(headers));

}

void fauxmoESP::_onTCPData(AsyncClient *client, unsigned int device_id, void *data, size_t len) {

    if (!_enabled) return;

    char setup[] = {"GET /setup.xml HTTP/1.1"};
    if (memcmp(data, setup, strlen(setup)-1) == 0) {
        _handleSetup(client, device_id);
    }

    char event[] = {"POST /upnp/control/basicevent1 HTTP/1.1"};
    if (memcmp(data, event, strlen(event)-1) == 0) {
        _handleContent(client, device_id, data, len);
    }

}

void fauxmoESP::_onTCPClient(AsyncClient *client, unsigned int device_id) {

    for (unsigned char i = 0; i < TCP_MAX_CLIENTS; i++) {

        if (!_tcpClients[i] || !_tcpClients[i]->connected()) {

            _tcpClients[i] = client;

            client->onAck([i](void *s, AsyncClient *c, size_t len, uint32_t time) {
            }, 0);

            client->onData([this, i, device_id](void *s, AsyncClient *c, void *data, size_t len) {
                _onTCPData(c, device_id, data, len);
            }, 0);

            client->onDisconnect([this, i](void *s, AsyncClient *c) {
                _tcpClients[i]->free();
                delete(_tcpClients[i]);
                DEBUG_MSG_FAUXMO("[FAUXMO] Client #%d disconnected\n", i);
            }, 0);

            client->onError([i](void *s, AsyncClient *c, int8_t error) {
                DEBUG_MSG_FAUXMO("[FAUXMO] Error %s (%d) on client #%d\n", c->errorToString(error), error, i);
            }, 0);

            client->onTimeout([i](void *s, AsyncClient *c, uint32_t time) {
                DEBUG_MSG_FAUXMO("[FAUXMO] Timeout on client #%d at %i\n", i, time);
                c->close();
            }, 0);

            DEBUG_MSG_FAUXMO("[FAUXMO] Client #%d connected\n", i);
            return;

        }
    }

    DEBUG_MSG_FAUXMO("[FAUXMO] Rejecting - Too many connections\n");
    client->onDisconnect([](void *s, AsyncClient *c) {
        delete(c);
    });
    client->stop();

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
    new_device.server = new AsyncServer(_base_port + device_id);
    new_device.server->onClient([this, device_id](void *s, AsyncClient* c) {
        _onTCPClient(c, device_id);
    }, 0);
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
        _onUDPData(remoteIP, remotePort, data, len);
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

    // Start UDP server on STA connection
    _handler = WiFi.onStationModeGotIP([this](WiFiEventStationModeGotIP ipInfo) {
        _udp.beginMulticast(WiFi.localIP(), UDP_MULTICAST_IP, UDP_MULTICAST_PORT);
        DEBUG_MSG_FAUXMO("[FAUXMO] UDP server started\n");
    });

}
