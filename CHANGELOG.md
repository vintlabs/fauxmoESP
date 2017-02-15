# fauxmoESP change log

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

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
