# fauxmoESP change log

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

## [3.2] 2020-12-22
### Changed
Fixed modelid so devices properly show as a light in the Alexa App

## [3.1.2] 2020-12-08
### Changed
- Changed `uniqueid` to fix Alexa discovery issue 
- Added `setDeviceUniqueId(unsigned char id, const char *uniqueid)` to optionally manually set a uniqueid

## [3.1.1] 2020-03-21
### Changed
- Freeze platformio.ini configuration

### Added 
- Add ssdp:discover to accepted UDP answers (#58, thanks to @rodymary)

## [3.1.0] 2018-11-18
### Fixed
- PlatformIO dependencies (#60)
- Fix issue with enable() call that prevented it from working the first time (#39)

### Changed
- Improved examples & documentation (#62, #65)
- Not using printf_P in ESP8266 since 2.3.0 does not support it (#69)

### Added
- Support for gen3 (third generation) Alexa devices (#66)
- Option to use external webserver to handle TCP requests
- RX timeout
- Free class resources upon object deletion
- Method to remove a device
- Method to rename a devices based on the old name (#57)
- Method to get a device_id from the name (#57)
- Method setState to report state changes to Alexa (#59)

## [3.0.2] 2018-10-08
### Fixed
- Removed symlinks to allow Arduino track the library

### Changed
- Debug messages moved to PROGMEME

## [3.0.1] 2018-08-29
### Fixed
- Possible segmentation fault in getDeviceName

## [3.0.0] 2018-08-15
### Changed
- Completely new paradigm, emulating Philips Hue API instead of Belkin Wemo API
- onSetState signature has changed to accommodate the brightness value
- onGetState no longer exists

### Added
- DEBUG_FAUXMO_VERBOSE setting for verbose debug output (defaults to false)

## [2.4.3] 2018-06-23
Dummy version to force PlatformIO parsing

## [2.4.2] 2018-01-14
### Fixed
- Removed a Serial.print

## [2.4.1] 2018-01-10
### Fixed
- Fixed dependency issue in PlatformIO (the hard way)

## [2.4.0] 2018-01-05
### Fixed
- Fixed examples for Arduino IDE (#34)

### Added
- Support for ESP32 (#28, thanks to Frank Hellmann)
- Preliminary support for Gen2 devices (thanks to Bibi Blocksberg)

## [2.3.0] 2017-11-08
### Fixed
- Answer correctly to GetBinaryState queries (thanks to John McC)

### Deprecated
- Use onSetState callback instead of onMessage callback
- Use onGetState callback instead of setState method

## [2.2.1] 2017-10-25
### Fixed
- Only change state if request is a SetBinaryState action

## [2.2.0] 2017-09-01
### Fixed
- Remove dependency on ESPAsyncWebServer

### Added
- Option to rename devices

## [2.1.1] 2017-08-25
### Fixed
- Call UDP beginMulticast on onStationModeGotIP

## [2.1.0] 2017-02-15
### Added
- ESPAsyncWebServer to manage TCP connections

### Fixed
- Memory leaks in UDP and TCP connections

## [2.0.0] 2017-01-05
### Added
- Different discovery strategies to maximize device discovery
- UDP polling required (call handle() in your loop)

### Changed
- ```onMessage``` callback signature changed to add device_id

### Removed
- Removed dependency on ESPAsyncUDP for compatibility with current stable release of Arduino Code for ESP8266

## [1.0.0] 2016-11-26
### Added
- Support for multiple virtual devices

### Changed
- ```onMessage``` callback signature has changed to allow passing the device name

### Deprecated
- ```setDeviceName``` no longer exists, use ```addDevice``` instead

## [0.1.0] 2016-11-21
- Initial working version
