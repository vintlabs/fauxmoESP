# fauxmoESP change log

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

## [2.0.0] 2017-01-03
### Added
- Compatibility mode (by default) allows you to compile the library against current stable release of Arduino Code for ESP8266
- UDP polling is required when in compatibility mode (by default)
- Using ESPAsyncWebServer for TCP requests

### Changed
- Callback signature changed to add device_id

## [1.0.0] 2016-11-26
### Added
- Support for multiple virtual devices

### Changed
- ```onMessage``` callback signature has changed to allow passing the device name

### Deprecated
- ```setDeviceName``` no longer exists, use ```addDevice``` instead

## [0.1.0] 2016-11-21
- Initial working version
