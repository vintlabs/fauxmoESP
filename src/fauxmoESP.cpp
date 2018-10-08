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

#include <Arduino.h>
#include "fauxmoESP.h"

// -----------------------------------------------------------------------------
// UDP
// -----------------------------------------------------------------------------

void fauxmoESP::_sendUDPResponse() {

	DEBUG_MSG_FAUXMO("[FAUXMO] Responding to M-SEARCH request\n");

	IPAddress ip = WiFi.localIP();
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    mac.toLowerCase();

	char response[strlen(FAUXMO_UDP_RESPONSE_TEMPLATE) + 128];
    snprintf_P(
        response, sizeof(response),
        FAUXMO_UDP_RESPONSE_TEMPLATE,
        ip[0], ip[1], ip[2], ip[3],
		TCP_PORT,
        mac.c_str(), mac.c_str()
    );

	#if DEBUG_FAUXMO_VERBOSE
    	DEBUG_MSG_FAUXMO("[FAUXMO] UDP response sent to %s:%u\n%s", _udp.remoteIP().toString().c_str(), _udp.remotePort(), response);
	#endif

    _udp.beginPacket(_udp.remoteIP(), _udp.remotePort());
	#if defined(ESP32)
	    _udp.printf(response);
	#else
	    _udp.write(response);
	#endif
    _udp.endPacket();

}

void fauxmoESP::_handleUDP() {

	int len = _udp.parsePacket();
    if (len > 0) {

		unsigned char data[len+1];
        _udp.read(data, len);
        data[len] = 0;

		#if DEBUG_FAUXMO_VERBOSE
			DEBUG_MSG_FAUXMO("[FAUXMO] UDP packet received\n%s", (const char *) data);
		#endif

        String request = (const char *) data;
        if (request.indexOf("M-SEARCH") >= 0) {
            if(request.indexOf("upnp:rootdevice") > 0 || request.indexOf("device:basic:1") > 0) {
                _sendUDPResponse();
            }
        }
    }

}


// -----------------------------------------------------------------------------
// TCP
// -----------------------------------------------------------------------------

void fauxmoESP::_sendTCPResponse(AsyncClient *client, const char * code, char * body, const char * mime) {

	char headers[strlen_P(FAUXMO_TCP_HEADERS) + 32];
	snprintf_P(
		headers, sizeof(headers),
		FAUXMO_TCP_HEADERS,
		code, mime, strlen(body)
	);

	#if DEBUG_FAUXMO_VERBOSE
		DEBUG_MSG_FAUXMO("[FAUXMO] Response:\n%s%s\n", headers, body);
	#endif

	client->write(headers);
	client->write(body);

}

String fauxmoESP::_deviceJson(unsigned char id) {

	if (id >= _devices.size()) return "{}";

	String mac = WiFi.macAddress();
    mac.replace(":", "");
    mac.toLowerCase();

	fauxmoesp_device_t device = _devices[id];
    char buffer[strlen_P(FAUXMO_DEVICE_JSON_TEMPLATE) + 64];
    snprintf_P(
        buffer, sizeof(buffer),
        FAUXMO_DEVICE_JSON_TEMPLATE,
        device.name, mac.c_str(), id+1,
        device.state ? "true": "false",
        device.value
    );

	return String(buffer);

}

void fauxmoESP::_onTCPDescription(AsyncClient *client, void *data, size_t len) {

    (void) data;
    (void) len;

	DEBUG_MSG_FAUXMO("[FAUXMO] Handling /description.xml request\n");

	IPAddress ip = WiFi.localIP();
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    mac.toLowerCase();

	char response[strlen_P(FAUXMO_DESCRIPTION_TEMPLATE) + 64];
    snprintf_P(
        response, sizeof(response),
        FAUXMO_DESCRIPTION_TEMPLATE,
        ip[0], ip[1], ip[2], ip[3], TCP_PORT,
        ip[0], ip[1], ip[2], ip[3], TCP_PORT,
        mac.c_str(), mac.c_str()
    );

	_sendTCPResponse(client, "200 OK", response, "text/xml");

}

void fauxmoESP::_onTCPList(AsyncClient *client, void *data, size_t len) {

	DEBUG_MSG_FAUXMO("[FAUXMO] Handling list request\n");

	char * p = (char *) data;
	p[len] = 0;
	String request = String(p);

	// Get the index
	int pos = request.indexOf("lights");
	unsigned char id = request.substring(pos+7).toInt();

	String response;

	// Client is requesting all devices
	if (0 == id) {

		response += "{";
		for (unsigned char i=0; i< _devices.size(); i++) {
			if (i>0) response += ",";
			response += "\"" + String(i+1) + "\":" + _deviceJson(i);
		}
		response += "}";

	// Client is requesting a single device
	} else {
		response = _deviceJson(id-1);
	}

	_sendTCPResponse(client, "200 OK", (char *) response.c_str(), "application/json");


}

void fauxmoESP::_onTCPControl(AsyncClient *client, void *data, size_t len) {

	char * p = (char *) data;
	p[len] = 0;
	String request = String(p);

	// "devicetype" request
	if (request.indexOf("devicetype") > 0) {
		DEBUG_MSG_FAUXMO("[FAUXMO] Handling devicetype request\n");
		_sendTCPResponse(client, "200 OK", (char *) "[{\"success\":{\"username\": \"2WLEDHardQrI3WHYTHoMcXHgEspsM8ZZRpSKtBQr\"}}]", "application/json");
		return;
	}

	// "state" request
	if (request.indexOf("state") > 0) {

		DEBUG_MSG_FAUXMO("[FAUXMO] Handling state request\n");

		// Get the index
		int pos = request.indexOf("lights");
		unsigned char id = request.substring(pos+7).toInt();

		if (id > 0) {

			--id;

			// Brightness
			pos = request.indexOf("bri");
			if (pos > 0) {
				unsigned char value = request.substring(pos+5).toInt();
				_devices[id].value = value;
				_devices[id].state = (value > 0);
			} else if (request.indexOf("false") > 0) {
				_devices[id].state = false;
			} else {
				_devices[id].state = true;
				if (0 == _devices[id].value) _devices[id].value = 255;
			}

			char response[strlen_P(FAUXMO_TCP_STATE_RESPONSE)+10];
			snprintf_P(
				response, sizeof(response),
				FAUXMO_TCP_STATE_RESPONSE,
				id+1, _devices[id].state ? "true" : "false", id+1, _devices[id].value
			);
			_sendTCPResponse(client, "200 OK", response, "text/xml");

			if (_setCallback) {
				_setCallback(id, _devices[id].name, _devices[id].state, _devices[id].value);
			}

			return;

		}

	}

	// Else return empty response
	_sendTCPResponse(client, "200 OK", (char *) "{}", "text/xml");

}

void fauxmoESP::_onTCPData(AsyncClient *client, void *data, size_t len) {

    if (!_enabled) return;

	#if DEBUG_FAUXMO_VERBOSE
    	char * p = (char *) data;
    	p[len] = 0;
		DEBUG_MSG_FAUXMO("[FAUXMO] TCP request\n%s\n", p);
	#endif

    {
        char match[] = {"GET /description.xml HTTP/1.1"};
        if (memcmp(data, match, strlen(match)-1) == 0) {
            _onTCPDescription(client, data, len);
            return;
        }
    }

    {
        char match[] = {"PUT /api/"};
        if (memcmp(data, match, strlen(match)-1) == 0) {
            _onTCPControl(client, data, len);
            return;
        }
    }

	{
        char match[] = {"GET /api/"};
        if (memcmp(data, match, strlen(match)-1) == 0) {
            _onTCPList(client, data, len);
            return;
        }
    }

}

void fauxmoESP::_onTCPClient(AsyncClient *client) {

	if (_enabled) {

	    for (unsigned char i = 0; i < TCP_MAX_CLIENTS; i++) {

	        if (!_tcpClients[i] || !_tcpClients[i]->connected()) {

	            _tcpClients[i] = client;

	            client->onAck([i](void *s, AsyncClient *c, size_t len, uint32_t time) {
	            }, 0);

	            client->onData([this, i](void *s, AsyncClient *c, void *data, size_t len) {
	                _onTCPData(c, data, len);
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

	} else {
		DEBUG_MSG_FAUXMO("[FAUXMO] Rejecting - Disabled\n");
	}

    client->onDisconnect([](void *s, AsyncClient *c) {
        c->free();
        delete c;
    });
    client->close(true);

}

// -----------------------------------------------------------------------------
// Devices
// -----------------------------------------------------------------------------

unsigned char fauxmoESP::addDevice(const char * device_name) {

    fauxmoesp_device_t device;
    unsigned int device_id = _devices.size();

    // init properties
    device.name = strdup(device_name);
	device.state = false;
	device.value = 0;

    // Attach
    _devices.push_back(device);

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
    if ((0 <= id) && (id <= _devices.size()) && (device_name != NULL)) {
        strncpy(device_name, _devices[id].name, len);
    }
    return device_name;
}

// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------

void fauxmoESP::handle() {
    if (_enabled) _handleUDP();
}

void fauxmoESP::enable(bool enable) {

	if (enable == _enabled) return;
    _enabled = enable;
	if (_enabled) {
		DEBUG_MSG_FAUXMO("[FAUXMO] Enabled\n");
	} else {
		DEBUG_MSG_FAUXMO("[FAUXMO] Disabled\n");
	}

    if (_enabled) {

		// Start TCP server
		if (NULL == _server) {
			_server = new AsyncServer(TCP_PORT);
	    	_server->onClient([this](void *s, AsyncClient* c) {
	        	_onTCPClient(c);
	    	}, 0);
		}
	    _server->begin();

		// UDP setup
		#ifdef ESP32
            _udp.beginMulticast(UDP_MULTICAST_IP, UDP_MULTICAST_PORT);
        #else
            _udp.beginMulticast(WiFi.localIP(), UDP_MULTICAST_IP, UDP_MULTICAST_PORT);
        #endif
        DEBUG_MSG_FAUXMO("[FAUXMO] UDP server started\n");

	}

}
