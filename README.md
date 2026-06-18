SonicEsp8266

Sonic animation and side-scrolling mini-game for ESP8266 (NodeMCU) and OLED SSD1306 display.
1. Requirements

    Hardware: * NodeMCU ESP8266

        OLED SSD1306 (128x64, I2C)

        3 Pushbuttons (Left, Right, Jump)

    Libraries: Adafruit SSD1306 and Adafruit GFX (via Arduino Library Manager).

    Setup: Rename all data .txt files to .h (e.g., intro.txt → intro_data.h) so the Arduino IDE includes them during compilation.

2. Wiring
Button	ESP8266 Pin
Jump	D3
Right	D5
Left	D4
Note: Connect the other side of all buttons to GND. Use INPUT_PULLUP in code.	
3. How it Works

    Memory: Bitmap arrays are stored in PROGMEM using namespaces to prevent variable conflicts.

    Logic:

        INTRO_NORMAL: Automated animation sequence.

        INTRO_INTERACTIVA: Static menu with blinking "PRESS JUMP".

        JUGANDO: Implements physics (gravity), 3-button movement (left/right/jump), and procedural obstacle scrolling.

    Non-blocking: Uses millis() for animation timing and button debouncing.

4. Code structure

    SonicEsp8266.ino: Main state machine and game physics.

    intro.h / sonic.h: Wrapper headers for animation namespaces.

    *_data.h: Raw bitmap arrays.
