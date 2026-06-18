# SonicEsp8266

<p align="center">
<img src="https://github.com/user-attachments/assets/33c21115-f03a-4614-88ca-55cdfffba49c" width="650">
</p>


SonicEsp8266 is a retro-style embedded game built on an ESP8266 and an SSD1306 OLED display.
Inspired by classic endless runner games, the player controls Sonic using physical push buttons, collecting rings while avoiding obstacles displayed on a 128x64 OLED screen.
The project demonstrates embedded programming, game logic implementation, OLED graphics rendering, and hardware integration using the ESP8266 platform.

## Stack

* ESP8266
* Arduino Framework
* C++
* I2C Communication Protocol
* SSD1306 OLED Display
* Arduino IDE

## Features

* OLED display initialization
* I2C communication setup
* Real-time text rendering
* Embedded systems programming
* Expandable architecture for sensors and IoT devices

## Hardware

* ESP8266 Development Board
* SSD1306 OLED Display (128x64)
* Push Buttons
* Breadboard
* USB Cable


## Wiring

| OLED | ESP8266 | 
| ---- | ------- |
| VCC  | 3.3V    |
| GND  | GND     |
| SDA  | D2      |
| SCL  | D1      |
|    BUTTONS     | 
| ---- | ------- |
| LEFT | D3      |
| RIGHT| D4      |
| JUMP | D5      |


## Prototype
<img src="https://github.com/user-attachments/assets/b20ff7c6-08ba-45e5-9f43-d258ccb73bc8" width="250">

## Installation

```bash
git clone https://github.com/mrppsi/SonicEsp8266.git
```

Install the required libraries:

* Adafruit SSD1306
* Adafruit GFX
* Wire

Upload the sketch to the ESP8266 using Arduino IDE.


Soon new updates and fixes sprites and physics. :) 

