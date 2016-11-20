/*

FAUXMO ESP

Copyright (C) 2016 by Xose Pérez <xose dot perez at gmail dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "fauxmoESP.h"

void fauxmoESP::_sayHello(IPAddress remoteIP, unsigned int port) {

    DEBUG_MSG_FAUXMO("[FAUXMO] Search request from %s\n", remoteIP.toString().c_str());

    AsyncUDP udpClient;
    if (udpClient.connect(remoteIP, port)) {

        char buffer[350];
        sprintf_P(buffer,
            HELLO_TEMPLATE,
            WiFi.localIP().toString().c_str(),
            _tcp_port,
            _uuid,
            DEVICE_PATTERN,
            _uuid,
            DEVICE_PATTERN
        );

        DEBUG_MSG_FAUXMO("[FAUXMO] Response (length=%d):\n", strlen(buffer));
        DEBUG_MSG_FAUXMO(buffer);

        udpClient.print(buffer);

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

void fauxmoESP::_handleTCPPacket(AsyncClient *client, void *data, size_t len) {

    char * message = (char *) data;

    unsigned int lenSetup = strlen(SETUP_PATTERN);
    unsigned int lenEvent = strlen(EVENT_PATTERN);
    unsigned int lenState = strlen(STATE_PATTERN);

    if (strncmp(message, SETUP_PATTERN, lenSetup) == 0) {

        DEBUG_MSG_FAUXMO("[FAUXMO] Got setup.xml request\n");

        char xml[350];
        sprintf_P(xml, XML_TEMPLATE, _device_name, _uuid);

        char buffer[600];
        sprintf_P(buffer, SETUP_TEMPLATE, strlen(xml), xml);

        DEBUG_MSG_FAUXMO("[FAUXMO] Response (length=%d):\n", strlen(buffer));
        DEBUG_MSG_FAUXMO(buffer);

        client->write(buffer);
        client->close();

    } else if (strncmp(message, EVENT_PATTERN, lenEvent) == 0) {

        DEBUG_MSG_FAUXMO("[FAUXMO] Got basicevent1 request\n");

        len -= lenState;
        for (int i = lenEvent; i < len; i++) {
            if (message[i] == STATE_PATTERN[0]) {
                if (strncmp(message + i, STATE_PATTERN, lenState) == 0) {

                    char state[2] = {0};
                    state[0] = message[i+lenState];
                    DEBUG_MSG_FAUXMO("[FAUXMO] State: %s\n", state);

                    if (_callback) _callback(state);

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

void fauxmoESP::_handleTCPClient(AsyncClient *client) {

    if (!_enabled) return;

    for (int i = 0; i < TCP_MAX_CLIENTS; i++) {
        if (!_clients[i] || !_clients[i]->connected()) {

            _clients[i] = client;

            client->onAck([this, i](void *s, AsyncClient *c, size_t len, uint32_t time) {
                DEBUG_MSG_FAUXMO("[FAUXMO] Got ack for client %i len=%u time=%u\n", i, len, time);
            }, 0);

            client->onData([this, i](void *s, AsyncClient *c, void *data, size_t len) {
                DEBUG_MSG_FAUXMO("[FAUXMO] Got data from client %i len=%i\n", i, len);
                _handleTCPPacket(c, data, len);
            }, 0);

            client->onDisconnect([this, i](void *s, AsyncClient *c) {
                DEBUG_MSG_FAUXMO("[FAUXMO] Disconnect for client %i\n", i);
                _clients[i]->free();
                delete(_clients[i]);
                _clients[i] = 0;
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

}

void fauxmoESP::setDeviceName(const char * device_name) {
    _device_name = strdup(device_name);
}

fauxmoESP::fauxmoESP(unsigned int port) : _tcp(port) {

    _tcp_port = port;
    sprintf(_uuid, "66617578%06X\0", ESP.getChipId()); // "FAUX" + CHIPID

    // UDP Server
    if (_udp.listenMulticast(UDP_MULTICAST_IP, UDP_MULTICAST_PORT)) {
        _udp.onPacket([this](AsyncUDPPacket packet) {
            _handleUDPPacket(packet);
        });
    }

    // TCP Server
    _tcp.onClient([this](void *s, AsyncClient* client) {
        _handleTCPClient(client);
    }, 0);
    _tcp.begin();

}
