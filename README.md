<img src="/img/clock.png" align="right" width="20%" />

# ESP8266-Seven-Segment-Clock


Required libraries: [EspMQTTClient](https://github.com/plapointe6/EspMQTTClient),
                    [OneWire](https://github.com/PaulStoffregen/OneWire),
                    [NTPClient](https://github.com/arduino-libraries/NTPClient)
                    [Timezone](https://github.com/JChristensen/Timezone),
                    [Time](https://github.com/PaulStoffregen/Time),
                    [SevenSegmentSR](https://github.com/Jan-Marvin/SevenSegmentSR)

Sketch of my smart clock.

## Features:
* NTP (with daylight saving)
* Controllable via MQTT
* Temperature Sensor (DS18B20)
