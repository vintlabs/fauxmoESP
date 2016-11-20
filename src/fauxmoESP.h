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

#ifndef FAUXMOESP_h
#define FAUXMOESP_h

#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncUDP.h>
#include <functional>

#define DEFAULT_TCP_PORT    52000
#define UDP_MULTICAST_IP    IPAddress(239,255,255,250)
#define UDP_MULTICAST_PORT  1900
#define TCP_MAX_CLIENTS     5

#define FAUXMO_ERROR_NONE   0x00
#define FAUXMO_ERROR_UDP    0x01
#define FAUXMO_ERROR_TCP    0x02

#define SEARCH_PATTERN      "M-SEARCH"
#define DEVICE_PATTERN      "urn:Belkin:device:**"
#define SETUP_PATTERN       "GET /setup.xml HTTP/1.1"
#define EVENT_PATTERN       "POST /upnp/control/basicevent1 HTTP/1.1"
#define STATE_PATTERN       "<BinaryState>"

#define HELLO_TEMPLATE      PSTR("HTTP/1.1 200 OK\r\nCACHE-CONTROL: max-age=86400\r\nDATE: Sun, 20 Nov 2016 00:00:00 GMT\r\nEXT:\r\nLOCATION: http://%s:%d/setup.xml\r\nOPT: \"http://schemas.upnp.org/upnp/1/0/\"; ns=01\r\n01-NLS: %s\r\nSERVER: Unspecified, UPnP/1.0, Unspecified\r\nST: %s\r\nUSN: uuid:Socket-1_0-%s::%s\r\n\r\n")
#define XML_TEMPLATE        PSTR("<?xml version=\"1.0\"?><root><device><deviceType>urn:MakerMusings:device:controllee:1</deviceType><friendlyName>%s</friendlyName><manufacturer>Belkin International Inc.</manufacturer><modelName>Emulated Socket</modelName><modelNumber>3.1415</modelNumber><UDN>uuid:Socket-1_0-%s</UDN></device></root>")
#define SETUP_TEMPLATE      PSTR("HTTP/1.1 200 OK\r\nCONTENT-LENGTH: %d\r\nCONTENT-TYPE: text/xml\r\nDATE: Sun, 20 Nov 2016 00:00:00 GMT\r\nLAST-MODIFIED: Sat, 01 Jan 2000 00:01:15 GMT\r\nSERVER: Unspecified, UPnP/1.0, Unspecified\r\nX-USER-AGENT: redsonic\r\nCONNECTION: close\r\n\r\n%s\r\n")
#define SOAP_TEMPLATE       PSTR("HTTP/1.1 200 OK\r\nCONTENT-LENGTH: 0\r\nCONTENT-TYPE: text/xml\r\nDATE: Sun, 20 Nov 2016 00:00:00 GMT\r\nEXT:\r\nSERVER: Unspecified, UPnP/1.0, Unspecified\r\nX-USER-AGENT: redsonic\r\nCONNECTION: close\r\n\r\n\r\n")

#ifdef DEBUG_FAUXMO
    #define DEBUG_MSG_FAUXMO(...) DEBUG_FAUXMO.printf( __VA_ARGS__ )
#else
    #define DEBUG_MSG_FAUXMO(...)
#endif

typedef std::function<void(const char *)> TStateFunction;

class fauxmoESP {

    public:

        fauxmoESP(unsigned int port = DEFAULT_TCP_PORT);
        void setDeviceName(const char * device_name);
        void onMessage(TStateFunction fn) { _callback = fn; }
        void enable(bool enable) { _enabled = enable; }

    private:

        bool _enabled = true;
        char _uuid[15];
        const char * _device_name;
        unsigned int _tcp_port;
        TStateFunction _callback = NULL;

        AsyncUDP _udp;
        AsyncServer _tcp;
        AsyncClient * _clients[TCP_MAX_CLIENTS];


        void _handleUDPPacket(AsyncUDPPacket packet);
        void _handleTCPPacket(AsyncClient *client, void *data, size_t len);
        void _handleTCPClient(AsyncClient *client);
        void _sayHello(IPAddress remoteIP, unsigned int port);

};

#endif
