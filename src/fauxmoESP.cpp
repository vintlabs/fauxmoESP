/*

FAUXMO ESP 2.4.2

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

#if defined(ESP32)
	#include <WiFi.h>
#elif defined(ESP8266)
	#include <ESP8266WiFi.h>
#else
	#error Platform not supported
#endif

#include "fauxmoESP.h"

// -----------------------------------------------------------------------------
// UDP
// -----------------------------------------------------------------------------

void fauxmoESP::_sendUDPResponse(unsigned int device_id) {

    fauxmoesp_device_t device = _devices[device_id];
    DEBUG_MSG_FAUXMO("[FAUXMO] UDP response for device #%d (%s)\n", _current, device.name);

    char buffer[16];
    IPAddress ip = WiFi.localIP();
    snprintf_P(buffer, sizeof(buffer), PSTR("%d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);

    char response[strlen(UDP_TEMPLATE) + 128];
    snprintf_P(response, sizeof(response), UDP_TEMPLATE,
        buffer,
        _base_port + _current,
        device.uuid,
        _udpPattern == 1 ? UDP_DEVICE_PATTERN_1 : _udpPattern == 2 ? UDP_DEVICE_PATTERN_2 : _udpPattern == 3 ? UDP_DEVICE_PATTERN_3 : _udpPattern == 4 ? UDP_DEVICE_PATTERN_4 : _udpPattern == 5 ? UDP_DEVICE_PATTERN_5 : UDP_ROOT_DEVICE,
        device.uuid,
        _udpPattern == 1 ? UDP_DEVICE_PATTERN_1 : _udpPattern == 2 ? UDP_ROOT_DEVICE : _udpPattern == 3 ? UDP_ROOT_DEVICE : _udpPattern == 4 ? UDP_ROOT_DEVICE : _udpPattern == 5 ? UDP_ROOT_DEVICE : UDP_ROOT_DEVICE
    );

    _udp.beginPacket(_remoteIP, _remotePort);
	#if defined(ESP32)
	    _udp.printf(response);
	#else
	    _udp.write(response);
	#endif
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
        _udpPattern = 0;
        if (strstr(p, UDP_DEVICE_PATTERN_1) != NULL) _udpPattern = 1;
        if (strstr(p, UDP_DEVICE_PATTERN_2) != NULL) _udpPattern = 2;
        if (strstr(p, UDP_DEVICE_PATTERN_3) != NULL) _udpPattern = 3;
		if (strstr(p, UDP_DEVICE_PATTERN_4) != NULL) _udpPattern = 4; // ssdp:all
		if (strstr(p, UDP_DEVICE_PATTERN_5) != NULL) _udpPattern = 5; // ssdpsearch:all
        if (strstr(p, UDP_ROOT_DEVICE) != NULL) _udpPattern = 6;      // upnp:rootdevice
        if (_udpPattern) {

            #ifdef DEBUG_FAUXMO
                char buffer[16];
                snprintf_P(buffer, sizeof(buffer), PSTR("%d.%d.%d.%d"), remoteIP[0], remoteIP[1], remoteIP[2], remoteIP[3]);
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

// -----------------------------------------------------------------------------
// TCP
// -----------------------------------------------------------------------------

void fauxmoESP::_handleSetup(AsyncClient *client, unsigned int device_id, void *data, size_t len) {

    (void) data;
    (void) len;

    DEBUG_MSG_FAUXMO("[FAUXMO] Device #%d /setup.xml\n", device_id);
    _devices[device_id].hit = true;
    fauxmoesp_device_t device = _devices[device_id];

    char response[strlen_P(SETUP_TEMPLATE) + strlen(device.name) + strlen(device.uuid) + strlen(device.serial)];
    snprintf_P(response, sizeof(response), SETUP_TEMPLATE, device.name, device.uuid, device.serial);

    char headers[strlen_P(HEADERS) + 10];
    snprintf_P(headers, sizeof(headers), HEADERS, strlen(response));

    client->write(headers, strlen(headers));
    client->write(response, strlen(response));

}

void fauxmoESP::_handleEventService(AsyncClient *client, unsigned int device_id, void *data, size_t len) {

    (void) data;
    (void) len;

    DEBUG_MSG_FAUXMO("[FAUXMO] Device #%d /eventservice.xml\n", device_id);

    char response[strlen_P(EVENTSERVICE_TEMPLATE)];
    snprintf_P(response, sizeof(response), EVENTSERVICE_TEMPLATE);

    char headers[strlen_P(HEADERS) + 10];
    snprintf_P(headers, sizeof(headers), HEADERS, strlen(response));

    client->write(headers, strlen(headers));
    client->write(response, strlen(response));

}

void fauxmoESP::_handleMetaInfoService(AsyncClient *client, unsigned int device_id, void *data, size_t len) {

    (void) data;
    (void) len;

    DEBUG_MSG_FAUXMO("[FAUXMO] Device #%d /metainfoservice.xml\n", device_id);

    char response[strlen_P(METAINFO_TEMPLATE)];
    snprintf_P(response, sizeof(response), METAINFO_TEMPLATE);

    char headers[strlen_P(HEADERS) + 10];
    snprintf_P(headers, sizeof(headers), HEADERS, strlen(response));

    client->write(headers, strlen(headers));
    client->write(response, strlen(response));

}

void fauxmoESP::_handleControl(AsyncClient *client, unsigned int device_id, void *data, size_t len) {

    DEBUG_MSG_FAUXMO("[FAUXMO] Device #%d /upnp/control/basicevent1\n", device_id);

    char content[len+1];
    memcpy(content, data, len);
    fauxmoesp_device_t device = _devices[device_id];

    // The default template is the one for GetBinaryState queries
    const char * response_template = GETSTATE_TEMPLATE;

    if (strstr(content, "SetBinaryState") != NULL) {

        if (strstr(content, "<BinaryState>0</BinaryState>") != NULL) {
            if (_setCallback) _setCallback(device_id, device.name, false);
        }

        if (strstr(content, "<BinaryState>1</BinaryState>") != NULL) {
            if (_setCallback) _setCallback(device_id, device.name, true);
        }

        // Use a specific response template for SetBinaryState action
        response_template = SETSTATE_TEMPLATE;

    }

    // Update current state
    if (_getCallback) device.state = _getCallback(device_id, device.name);

    // Send response
    char response[strlen_P(response_template) + 10];
    snprintf_P(response, sizeof(response), response_template, device.state ? 1 : 0);

    char headers[strlen_P(HEADERS) + 10];
    snprintf_P(headers, sizeof(headers), HEADERS, strlen(response));

    client->write(headers, strlen(headers));
    client->write(response, strlen(response));

}

void fauxmoESP::_onTCPData(AsyncClient *client, unsigned int device_id, void *data, size_t len) {

    if (!_enabled) return;

    /*
    char * p = (char *) data;
    p[len] = 0;
    Serial.print(p);
    */

    {
        char match[] = {"GET /setup.xml HTTP/1.1"};
        if (memcmp(data, match, strlen(match)-1) == 0) {
            _handleSetup(client, device_id, data, len);
            return;
        }
    }

    {
        char match[] = {"GET /eventservice.xml HTTP/1.1"};
        if (memcmp(data, match, strlen(match)-1) == 0) {
            _handleEventService(client, device_id, data, len);
            return;
        }
    }

    {
        char match[] = {"GET /metainfoservice.xml HTTP/1.1"};
        if (memcmp(data, match, strlen(match)-1) == 0) {
            _handleMetaInfoService(client, device_id, data, len);
            return;
        }
    }

    {
        char match[] = {"POST /upnp/control/basicevent1 HTTP/1.1"};
        if (memcmp(data, match, strlen(match)-1) == 0) {
            _handleControl(client, device_id, data, len);
            return;
        }
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
                _tcpClients[i] = NULL;
                delete c;
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
        c->free();
        delete c;
    });
    client->close(true);

}

// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------

unsigned char fauxmoESP::addDevice(const char * device_name) {

    fauxmoesp_device_t new_device;
    unsigned int device_id = _devices.size();

    // Copy name
    new_device.name = strdup(device_name);

    // Chip ID
    #if defined(ESP32)
        unsigned long chip_id = (uint32_t) ESP.getEfuseMac();
    #else
        unsigned long chip_id = ESP.getChipId();
    #endif

    // Create UUID
    char uuid[15];
    snprintf_P(uuid, sizeof(uuid), PSTR("444556%06X%02X\0"), chip_id, device_id); // "DEV" + CHIPID + DEV_ID
    new_device.uuid = strdup(uuid);

    // Create Serialnumber
    char serial[15];
    sprintf(serial, "221703K0%06X\0", chip_id); // "221703K0" + CHIPID
    new_device.serial = strdup(serial);

    // TCP Server
    new_device.server = new AsyncServer(_base_port + device_id);
    new_device.server->onClient([this, device_id](void *s, AsyncClient* c) {
        _onTCPClient(c, device_id);
    }, 0);
    new_device.server->begin();

    // Attach
    _devices.push_back(new_device);

    DEBUG_MSG_FAUXMO("[FAUXMO] Device '%s' added as #%d\n", device_name, device_id);

    return device_id;

}

bool fauxmoESP::renameDevice(unsigned char id, const char * device_name) {
    if (0 <= id && id <= _devices.size()) {
        free(_devices[id].name);
        _devices[id].name = strdup(device_name);
        DEBUG_MSG_FAUXMO("[FAUXMO] Device #%d renamed to '%s'\n", id, device_name);
        return true;
    }
    return false;
}

char * fauxmoESP::getDeviceName(unsigned char id, char * device_name, size_t len) {
    if (0 <= id && id <= _devices.size()) {
        strncpy(device_name, _devices[id].name, len);
    }
    return device_name;
}

void fauxmoESP::setState(unsigned char id, bool state) {
    if (0 <= id && id <= _devices.size()) {
        _devices[id].state = state;
    }
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
	#ifdef ESP32
    	_udp.beginMulticast(UDP_MULTICAST_IP, UDP_MULTICAST_PORT);
	#endif
}

fauxmoESP::fauxmoESP(unsigned int port) {

    _base_port = port;

	#ifdef ESP8266
		// Start UDP server on STA connection
		_handler = WiFi.onStationModeGotIP([this](WiFiEventStationModeGotIP ipInfo) {
		    _udp.beginMulticast(WiFi.localIP(), UDP_MULTICAST_IP, UDP_MULTICAST_PORT);
		    DEBUG_MSG_FAUXMO("[FAUXMO] UDP server started\n");
		});
	#endif

}
