## Introduction

This sketch is created for Platform.io in Visual Studio Code / Code.

It can be hard to start a sketch from start, if you're limited on time.

Here's a sketch where I'm using the following functions/features:
    - Serial
    - Wifi
        - NTP Time 
    - Screen
    - Button
    - Loops with multiple timings

The solution uses the following libs:
    - NTPClient (Time over Wifi)
    - U8g2 (Screen-driver)
    - ArduinoJson (Parse JSON)
    - ezButton (Human-friendly button logics)

The solution has been tested with the following micro-controllers:
    - WeMos D1 mini Pro (Change `board` to `d1_mini_pro` in `platformio.ini`)
    - NodeMCU CP2102 and NodeMCU CH340 (Change `board` to `nodemcu` in `platformio.ini`)

The solution has been tested with the following screen(s):
    - SH1106_128X64 (Like the one you can find [here](https://www.aliexpress.com/item/32896971385.html)) - Connected to GND+3.3V+SCK+SDA

GPIO Outputs:
    - Button on GPIO2 (D4)
    - Screen SCK/SCL on GPIO5 (D1)
    - Screen SDA on GPIO4 (D2)

I'll guess it will work with other ESP-related devices too, if `espressif8266` is supported. I've personally used the `arduino` framework, and configured PlatformIO to use my Arduino CLI.

If you want higher upload speed, adjust the `upload_speed` in `platformio.ini`.


# References and Thanks to

- [WifiManager](https://github.com/tzapu/WiFiManager)
- [U8g2](https://github.com/olikraus/u8g2)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
- [ezButton](https://github.com/ArduinoGetStarted/button)