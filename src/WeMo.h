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

#pragma once

const char UDP_TEMPLATE[] PROGMEM =
    "HTTP/1.1 200 OK\r\n"
    "CACHE-CONTROL: max-age=86400\r\n"
    "DATE: Mon, 22 Jun 2015 17:24:01 GMT\r\n"
    "EXT:\r\n"
    "LOCATION: http://%s:%d/setup.xml\r\n"
    "OPT: \"http://schemas.upnp.org/upnp/1/0/\"; ns=01\r\n"
    "01-NLS: %s\r\n"
    "SERVER: Unspecified, UPnP/1.0, Unspecified\r\n"
    "X-User-Agent: redsonic\r\n"
    "ST: %s\r\n"
    "USN: uuid:Socket-1_0-%s::%s\r\n\r\n";

const char SETUP_TEMPLATE[] PROGMEM =
    "<?xml version=\"1.0\"?>"
    "<root>"
        "<specVersion><major>1</major><minor>0</minor></specVersion>"
        "<device>"
            "<deviceType>urn:Belkin:device:controllee:1</deviceType>"
            "<friendlyName>%s</friendlyName>"
            "<manufacturer>Belkin International Inc.</manufacturer>"
            "<modelName>Socket</modelName>"
            "<modelNumber>1.0</modelNumber>"
            "<UDN>uuid:Socket-1_0-%s</UDN>"
            "<serviceList>"
                "<service>"
                    "<serviceType>urn:Belkin:service:basicevent:1</serviceType>"
                    "<serviceId>urn:Belkin:serviceId:basicevent1</serviceId>"
                    "<controlURL>/upnp/control/basicevent1</controlURL>"
                    "<eventSubURL>/upnp/event/basicevent1</eventSubURL>"
                    "<SCPDURL>/eventservice.xml</SCPDURL>"
                "</service>"
                "<service>"
                    "<serviceType>urn:Belkin:service:metainfo:1</serviceType>"
                    "<serviceId>urn:Belkin:serviceId:metainfo1</serviceId>"
                    "<controlURL>/upnp/control/metainfo1</controlURL>"
                    "<eventSubURL>/upnp/event/metainfo1</eventSubURL>"
                    "<SCPDURL>/metainfoservice.xml</SCPDURL>"
                "</service>"
            "</serviceList>"
        "</device>"
    "</root>";

const char EVENTSERVICE_TEMPLATE[] PROGMEM =
    "<?xml version=\"1.0\"?>"
    "<scpd xmlns=\"urn:Belkin:service-1-0\">"
        "<specVersion><major>1</major><minor>0</minor></specVersion>"
        "<actionList>"
            "<action>"
                "<name>SetBinaryState</name>"
                "<argumentList>"
                    "<argument>"
                        "<retval />"
                        "<name>BinaryState</name>"
                        "<relatedStateVariable>BinaryState</relatedStateVariable>"
                        "<direction>in</direction>"
                    "</argument>"
                "</argumentList>"
            "</action>"
            "<action>"
                "<name>GetBinaryState</name>"
                "<argumentList>"
                    "<argument>"
                        "<retval/>"
                        "<name>BinaryState</name>"
                        "<relatedStateVariable>BinaryState</relatedStateVariable>"
                        "<direction>out</direction>"
                    "</argument>"
                "</argumentList>"
            "</action>"
        "</actionList>"
        "<serviceStateTable>"
            "<stateVariable sendEvents=\"yes\">"
                "<name>BinaryState</name>"
                "<dataType>Boolean</dataType>"
                "<defaultValue>0</defaultValue>"
            "</stateVariable>"
        "</serviceStateTable>"
    "</scpd>";

const char EMPTYSERVICE_TEMPLATE[] PROGMEM =
    "<?xml version=\"1.0\"?>"
    "<scpd xmlns=\"urn:Belkin:service-1-0\">"
        "<specVersion><major>1</major><minor>0</minor></specVersion>"
        "<actionList></actionList>"
        "<serviceStateTable></serviceStateTable>"
    "</scpd>";

const char SETSTATE_TEMPLATE[] PROGMEM =
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
    	"<s:Body>"
        	"<u:SetBinaryState xmlns:u=\"urn:Belkin:service:basicevent:1\">"
        	   "<BinaryState>%d</BinaryState>"
        	"</u:SetBinaryState>"
    	"</s:Body>"
	"</s:Envelope>";

const char GETSTATE_TEMPLATE[] PROGMEM =
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
        "<s:Body>"
            "<u:GetBinaryStateResponse xmlns:u=\"urn:Belkin:service:basicevent:1\">"
                "<BinaryState>%d</BinaryState>"
            "</u:GetBinaryStateResponse>"
        "</s:Body>"
    "</s:Envelope>";

const char HEADERS[] PROGMEM =
    "HTTP/1.1 200 OK\r\n"
    "CONTENT-LENGTH: %d\r\n"
    "CONTENT-TYPE: text/xml\r\n"
    "DATE: Sun, 01 Jan 2017 00:00:00 GMT\r\n"
    "LAST-MODIFIED: Sat, 01 Jan 2017 00:00:00 GMT\r\n"
    "SERVER: Unspecified, UPnP/1.0, Unspecified\r\n"
    "X-USER-AGENT: redsonic\r\n"
    "CONNECTION: close\r\n\r\n";
