# Why this boilerpack?

It can be hard to start a sketch from start, if you're limited on time. I've tried to get a boilerpack ready for you, which I personally use in my projects. It can be stripped down, and I'll try and improve it a bit over time.

The Wifi is configured on your phone, and the chip will start the hotspot `AutoConnectAP`by itself, thanks to WifiManger. Use the password `boilerpack`.

Take a look around the fine, and let me know if you have any suggestions. I've also tried and keep most of my ways to convert strings to integeres and so on, including a few says to do that.

## Introduction

This sketch is created for Platform.io in Visual Studio Code / Code.

Here's a sketch where I'm using the following functions/features:

- Serial - Both Commands and Print
- Wifi - Including Wifi Manger, get NTP Time and more
- Screen
- Button
- Loops with timings

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

# How to start

TBD...

# References and Thanks to

- [WifiManager](https://github.com/tzapu/WiFiManager)
- [U8g2](https://github.com/olikraus/u8g2)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
- [ezButton](https://github.com/ArduinoGetStarted/button)
