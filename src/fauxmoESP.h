/*

FAUXMO ESP

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

*/

#pragma once

#define UDP_MULTICAST_IP        IPAddress(239,255,255,250)
#define UDP_MULTICAST_PORT      1900
#define TCP_MAX_CLIENTS         10
#define TCP_PORT                1901

#ifdef DEBUG_FAUXMO
    #define DEBUG_MSG_FAUXMO(fmt, ...) { static const char pfmt[] PROGMEM = fmt; DEBUG_FAUXMO.printf_P(pfmt, ## __VA_ARGS__); }
#else
    #define DEBUG_MSG_FAUXMO(...)
#endif

#ifndef DEBUG_FAUXMO_VERBOSE
#define DEBUG_FAUXMO_VERBOSE    false
#endif

#include <Arduino.h>

#if defined(ESP8266)
    #include <ESP8266WiFi.h>
    #include <ESPAsyncTCP.h>
#elif defined(ESP32)
    #include <WiFi.h>
    #include <AsyncTCP.h>
#else
	#error Platform not supported
#endif

#include <WiFiUdp.h>
#include <functional>
#include <vector>
#include <templates.h>

typedef std::function<void(unsigned char, const char *, bool, unsigned char)> TSetStateCallback;

typedef struct {
    char * name;
    bool state;
    unsigned char value;
} fauxmoesp_device_t;

class fauxmoESP {

    public:

        unsigned char addDevice(const char * device_name);
        bool renameDevice(unsigned char id, const char * device_name);
        char * getDeviceName(unsigned char id, char * buffer, size_t len);
        void onSetState(TSetStateCallback fn) { _setCallback = fn; }
        void enable(bool enable);
        void handle();

    private:

        AsyncServer * _server;
        bool _enabled = true;
        std::vector<fauxmoesp_device_t> _devices;
		#ifdef ESP8266
        WiFiEventHandler _handler;
		#endif
        WiFiUDP _udp;
        AsyncClient * _tcpClients[TCP_MAX_CLIENTS];
        TSetStateCallback _setCallback = NULL;

        String _deviceJson(unsigned char id);

        void _handleUDP();
        void _onUDPData(const IPAddress remoteIP, unsigned int remotePort, void *data, size_t len);
        void _sendUDPResponse();

        void _onTCPClient(AsyncClient *client);
        void _onTCPData(AsyncClient *client, void *data, size_t len);
        void _onTCPDescription(AsyncClient *client, void *data, size_t len);
        void _onTCPList(AsyncClient *client, void *data, size_t len);
        void _onTCPControl(AsyncClient *client, void *data, size_t len);
        void _sendTCPResponse(AsyncClient *client, const char * code, char * body, const char * mime);

};
