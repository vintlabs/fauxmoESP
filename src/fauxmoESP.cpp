/*

FAUXMO ESP 1.0.0

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

void fauxmoESP::_sayHello(IPAddress remoteIP, unsigned int port) {

    DEBUG_MSG_FAUXMO("[FAUXMO] Search request from %s\n", remoteIP.toString().c_str());

    AsyncUDP udpClient;
    if (udpClient.connect(remoteIP, port)) {

        for (unsigned int i = 0; i < _devices.size(); i++) {

            fauxmoesp_device_t device = _devices[i];

            char buffer[350];
            sprintf_P(buffer,
                HELLO_TEMPLATE,
                WiFi.localIP().toString().c_str(),
                _base_port + i,
                device.uuid,
                DEVICE_PATTERN,
                device.uuid,
                DEVICE_PATTERN
            );

            DEBUG_MSG_FAUXMO("[FAUXMO] Response (length=%d):\n", strlen(buffer));
            DEBUG_MSG_FAUXMO(buffer);

            udpClient.print(buffer);

        }

    }

}

void fauxmoESP::_handleUDPPacket(AsyncUDPPacket packet) {

    if (!_enabled) return;

    //DEBUG_MSG_FAUXMO("[FAUXMO] Got UDP packet from %s\n", packet.remoteIP().toString().c_str());

    unsigned char * data = packet.data();
    unsigned int lenSearch = strlen(SEARCH_PATTERN);
    unsigned int lenDevice = strlen(DEVICE_PATTERN);
    unsigned int len = packet.length() - lenDevice + 1;

    if (strncmp((char *) data, SEARCH_PATTERN, lenSearch) == 0) {
        for (int i = lenSearch; i < len; i++) {
            if (data[i] == DEVICE_PATTERN[0]) {
                if (strncmp((char *) (data + i), DEVICE_PATTERN, lenDevice) == 0) {
                    _sayHello(packet.remoteIP(), packet.remotePort());
                    break;
                }
            }
        }
    }

}

void fauxmoESP::_handleTCPPacket(unsigned int device_id, AsyncClient *client, void *data, size_t len) {

    char * message = (char *) data;

    fauxmoesp_device_t device = _devices[device_id];

    unsigned int lenSetup = strlen(SETUP_PATTERN);
    unsigned int lenEvent = strlen(EVENT_PATTERN);
    unsigned int lenState = strlen(STATE_PATTERN);

    if (strncmp(message, SETUP_PATTERN, lenSetup) == 0) {

        DEBUG_MSG_FAUXMO("[FAUXMO] Got setup.xml request\n");

        char xml[350];
        sprintf_P(xml, XML_TEMPLATE, device.name, device.uuid);

        char buffer[600];
        sprintf_P(buffer, SETUP_TEMPLATE, strlen(xml), xml);

        DEBUG_MSG_FAUXMO("[FAUXMO] Response (length=%d):\n", strlen(buffer));
        DEBUG_MSG_FAUXMO(buffer);

        client->write(buffer);
        client->close();

    } else if (strncmp(message, EVENT_PATTERN, lenEvent) == 0) {

        DEBUG_MSG_FAUXMO("[FAUXMO] Got basicevent1 request\n");

        len = len - lenState - 1;
        for (int i = lenEvent; i < len; i++) {
            if (message[i] == STATE_PATTERN[0]) {
                if (strncmp(message + i, STATE_PATTERN, lenState) == 0) {

                    bool state = (message[i+lenState] == '1');
                    DEBUG_MSG_FAUXMO("[FAUXMO] %s state: %s\n", device.name, state ? "ON" : "OFF");

                    if (_callback) _callback(device.name, state);

                    break;

                }
            }
        }

        char buffer[600];
        sprintf_P(buffer, SOAP_TEMPLATE);

        DEBUG_MSG_FAUXMO("[FAUXMO] Response (length=%d):\n", strlen(buffer));
        DEBUG_MSG_FAUXMO(buffer);

        client->write(buffer);
        client->close();

    }

}

AcConnectHandler fauxmoESP::_getTCPClientHandler(unsigned int device_id) {

    return [this, device_id](void *s, AsyncClient * client) {

        if (!_enabled) return;

        for (int i = 0; i < TCP_MAX_CLIENTS; i++) {
            if (!_clients[i] || !_clients[i]->connected()) {

                _clients[i] = client;

                client->onAck([this, i](void *s, AsyncClient *c, size_t len, uint32_t time) {
                    DEBUG_MSG_FAUXMO("[FAUXMO] Got ack for client %i len=%u time=%u\n", i, len, time);
                }, 0);

                client->onData([this, i, device_id](void *s, AsyncClient *c, void *data, size_t len) {
                    DEBUG_MSG_FAUXMO("[FAUXMO] Got data from client %i len=%i\n", i, len);
                    _handleTCPPacket(device_id, c, data, len);
                }, 0);

                client->onDisconnect([this, i](void *s, AsyncClient *c) {
                    DEBUG_MSG_FAUXMO("[FAUXMO] Disconnect for client %i\n", i);
                    _clients[i]->free();
                }, 0);
                client->onError([this, i](void *s, AsyncClient *c, int8_t error) {
                    DEBUG_MSG_FAUXMO("[FAUXMO] Error %s (%i) on client %i\n", c->errorToString(error), error, i);
                }, 0);
                client->onTimeout([this, i](void *s, AsyncClient *c, uint32_t time) {
                    DEBUG_MSG_FAUXMO("[FAUXMO] Timeout on client %i at %i\n", i, time);
                    c->close();
                }, 0);

                return;

            }
        }

        DEBUG_MSG_FAUXMO("[FAUXMO] Rejecting client - Too many connections already.\n");

        // We cannot accept this connection at the moment
        client->onDisconnect([](void *s, AsyncClient *c) {
            delete(c);
        });
        client->stop();

    };

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
    new_device.server->onClient(_getTCPClientHandler(device_id), 0);
    new_device.server->begin();

    // Attach
    _devices.push_back(new_device);

}

fauxmoESP::fauxmoESP(unsigned int port) {

    _base_port = port;

    // UDP Server
    if (_udp.listenMulticast(UDP_MULTICAST_IP, UDP_MULTICAST_PORT)) {
        _udp.onPacket([this](AsyncUDPPacket packet) {
            _handleUDPPacket(packet);
        });
    }

}
