# CupActive
## Purpose
CupActive is an open source project created by sooldongmu team to enable 'Joyful Soolsang' project.
The purpose of the project is to show in 'Maker Faire Seoul 2018'.
CupActive repository would be in charge of each device software and CupActive-Server would be server software.
CupActive will handle hardware resources which are included in personal device.

## Hardware
Here is a list which hardwares are included in CupActive.
* Node MCU V3
* Neo Pixel Strip LED(WS2812 RGB)
* LoadCell 500g and HX711
* Vibration Motor / Driver(DRV2605)
* Buzzer
* 8x8 Dot Matrix and driver(MAX7219)
* 18650 rechargeble battery
* 1000C Power boost
* Micro USB PCB

## Software
Here is a list which external libraries are installed in CupActive.
* Thread
* Led Control(Dot Matrix)
* Adafruit NeoPixel
* WebSocket
* HX711
* ArduinoJson
* DRV2605

### How To Install
#### NodeMCU V3 Board Library
Add to the below link into additional board manager menu in preperence of file tab.
http://arduino.esp8266.com/stable/package_esp8266com_index.json </br>
and then go to the tool > board manager to find out esp8266 item. </br>
Click to install button and it will be fininshed in 5 minute to install.</br>
Please visit to the site If you get some huddle.</br>
https://www.instructables.com/id/Quick-Start-to-Nodemcu-ESP8266-on-Arduino-IDE/ </br>
If you have done to install it, you can see the Node MCU 1.0(ESP-12E module)
#### Load cell(HX711) library
The library has to be manually installed. Once take the source from the another git hub and copy the zip file in your pc.</br>
https://github.com/sparkfun/HX711-Load-Cell-Amplifier </br>
After downloading the zip file and open library manager to include it into your Arduino IDE.
#### The others
ArduinoJson, WebSocket, NeoPixel, Led Controller, thread and DRV2605 are much simple.</br>
Open library manager and find it to install.
* ArduinoThread by Ivan Seidel
* Adafruit DRV2605 Library by Adafruit
* Adafruit NeoPixel by Adafruit
* LedControl by Eberhard Fahle
* WebSockets by Markus Sattler


