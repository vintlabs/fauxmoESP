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

#ifndef FAUXMOESP_h
#define FAUXMOESP_h

#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncUDP.h>
#include <functional>
#include <vector>

#define DEFAULT_TCP_BASE_PORT    52000
#define UDP_MULTICAST_IP        IPAddress(239,255,255,250)
#define UDP_MULTICAST_PORT      1900
#define TCP_MAX_CLIENTS         5

#define FAUXMO_ERROR_NONE       0x00
#define FAUXMO_ERROR_UDP        0x01
#define FAUXMO_ERROR_TCP        0x02

#define SEARCH_PATTERN          "M-SEARCH"
#define DEVICE_PATTERN          "urn:Belkin:device:**"
#define SETUP_PATTERN           "GET /setup.xml HTTP/1.1"
#define EVENT_PATTERN           "POST /upnp/control/basicevent1 HTTP/1.1"
#define STATE_PATTERN           "<BinaryState>"

#define HELLO_TEMPLATE          PSTR("HTTP/1.1 200 OK\r\nCACHE-CONTROL: max-age=86400\r\nDATE: Sun, 20 Nov 2016 00:00:00 GMT\r\nEXT:\r\nLOCATION: http://%s:%d/setup.xml\r\nOPT: \"http://schemas.upnp.org/upnp/1/0/\"; ns=01\r\n01-NLS: %s\r\nSERVER: Unspecified, UPnP/1.0, Unspecified\r\nST: %s\r\nUSN: uuid:Socket-1_0-%s::%s\r\n\r\n")
#define XML_TEMPLATE            PSTR("<?xml version=\"1.0\"?><root><device><deviceType>urn:MakerMusings:device:controllee:1</deviceType><friendlyName>%s</friendlyName><manufacturer>Belkin International Inc.</manufacturer><modelName>Emulated Socket</modelName><modelNumber>3.1415</modelNumber><UDN>uuid:Socket-1_0-%s</UDN></device></root>")
#define SETUP_TEMPLATE          PSTR("HTTP/1.1 200 OK\r\nCONTENT-LENGTH: %d\r\nCONTENT-TYPE: text/xml\r\nDATE: Sun, 20 Nov 2016 00:00:00 GMT\r\nLAST-MODIFIED: Sat, 01 Jan 2000 00:01:15 GMT\r\nSERVER: Unspecified, UPnP/1.0, Unspecified\r\nX-USER-AGENT: redsonic\r\nCONNECTION: close\r\n\r\n%s\r\n")
#define SOAP_TEMPLATE           PSTR("HTTP/1.1 200 OK\r\nCONTENT-LENGTH: 0\r\nCONTENT-TYPE: text/xml\r\nDATE: Sun, 20 Nov 2016 00:00:00 GMT\r\nEXT:\r\nSERVER: Unspecified, UPnP/1.0, Unspecified\r\nX-USER-AGENT: redsonic\r\nCONNECTION: close\r\n\r\n\r\n")

#ifdef DEBUG_FAUXMO
    #define DEBUG_MSG_FAUXMO(...) DEBUG_FAUXMO.printf( __VA_ARGS__ )
#else
    #define DEBUG_MSG_FAUXMO(...)
#endif

typedef std::function<void(const char *, bool)> TStateFunction;

typedef struct {
    char * name;
    char * uuid;
    AsyncServer * server;
} fauxmoesp_device_t;

class fauxmoESP {

    public:

        fauxmoESP(unsigned int port = DEFAULT_TCP_BASE_PORT);
        void addDevice(const char * device_name);
        void onMessage(TStateFunction fn) { _callback = fn; }
        void enable(bool enable) { _enabled = enable; }

    private:

        bool _enabled = true;
        unsigned int _base_port = DEFAULT_TCP_BASE_PORT;
        std::vector<fauxmoesp_device_t> _devices;
        AsyncUDP _udp;
        AsyncClient * _clients[TCP_MAX_CLIENTS];
        TStateFunction _callback = NULL;

        void _handleUDPPacket(AsyncUDPPacket packet);
        AcConnectHandler _getTCPClientHandler(unsigned int device_id);
        void _handleTCPPacket(unsigned int device_id, AsyncClient *client, void *data, size_t len);
        void _sayHello(IPAddress remoteIP, unsigned int port);

};

#endif
