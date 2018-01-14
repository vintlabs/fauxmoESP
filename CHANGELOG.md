# fauxmoESP change log

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

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
