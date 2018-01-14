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

#pragma once

#define DEFAULT_TCP_BASE_PORT   52000
#define UDP_MULTICAST_IP        IPAddress(239,255,255,250)
#define UDP_MULTICAST_PORT      1900
#define TCP_MAX_CLIENTS         10

#define UDP_SEARCH_PATTERN      "M-SEARCH"
#define UDP_DEVICE_PATTERN_1    "urn:Belkin:device:**"
#define UDP_DEVICE_PATTERN_2    "urn:Belkin:device:controllee:1"
#define UDP_DEVICE_PATTERN_3    "urn:Belkin:service:basicevent:1"
#define UDP_DEVICE_PATTERN_4    "ssdp:all"
#define UDP_DEVICE_PATTERN_5    "ssdpsearch:all"
#define UDP_ROOT_DEVICE         "upnp:rootdevice"

#define UDP_RESPONSES_INTERVAL  250
#define UDP_RESPONSES_TRIES     5

#ifdef DEBUG_FAUXMO
    #define DEBUG_MSG_FAUXMO(...) DEBUG_FAUXMO.printf( __VA_ARGS__ )
#else
    #define DEBUG_MSG_FAUXMO(...)
#endif

#include <Arduino.h>

#if defined(ESP32)
	#include <WiFi.h>
	#include <AsyncTCP.h>
#elif defined(ESP8266)
	#include <ESP8266WiFi.h>
	#include <ESPAsyncTCP.h>
#else
	#error Platform not supported
#endif

#include <WiFiUdp.h>
#include <functional>
#include <vector>
#include <WeMo.h>

typedef std::function<void(unsigned char, const char *, bool)> TSetStateCallback;
typedef std::function<bool(unsigned char, const char *)> TGetStateCallback;

typedef struct {
    char * name;
    char * uuid;
	char * serial;
    bool hit;
    bool state;
    AsyncServer * server;
} fauxmoesp_device_t;

class fauxmoESP {

    public:

        fauxmoESP(unsigned int port = DEFAULT_TCP_BASE_PORT);

        unsigned char addDevice(const char * device_name);
        bool renameDevice(unsigned char id, const char * device_name);
        char * getDeviceName(unsigned char id, char * buffer, size_t len);
        void onSetState(TSetStateCallback fn) { _setCallback = fn; }
        void onGetState(TGetStateCallback fn) { _getCallback = fn; }
        void enable(bool enable);
        void handle();

        // backwards compatibility DEPRECATED
        void onMessage(TSetStateCallback fn) { onSetState(fn); }
        void setState(unsigned char id, bool state);

    private:

        bool _enabled = true;
        unsigned int _base_port = DEFAULT_TCP_BASE_PORT;
        std::vector<fauxmoesp_device_t> _devices;
		#ifdef ESP8266
        WiFiEventHandler _handler;
		#endif
        WiFiUDP _udp;
        AsyncClient * _tcpClients[TCP_MAX_CLIENTS];
        TSetStateCallback _setCallback = NULL;
        TGetStateCallback _getCallback = NULL;

        unsigned int _roundsLeft = 0;
        unsigned int _current = 0;
        unsigned long _lastTick;
        IPAddress _remoteIP;
        unsigned int _remotePort;
        unsigned int _udpPattern;

        void _sendUDPResponse(unsigned int device_id);
        void _nextUDPResponse();

        void _handleSetup(AsyncClient *client, unsigned int device_id, void *data, size_t len);
        void _handleMetaInfoService(AsyncClient *client, unsigned int device_id, void *data, size_t len);
        void _handleEventService(AsyncClient *client, unsigned int device_id, void *data, size_t len);
        void _handleControl(AsyncClient *client, unsigned int device_id, void *data, size_t len);

        void _onUDPData(const IPAddress remoteIP, unsigned int remotePort, void *data, size_t len);
        void _onTCPData(AsyncClient *client, unsigned int device_id, void *data, size_t len);
        void _onTCPClient(AsyncClient *client, unsigned int device_id);

};
