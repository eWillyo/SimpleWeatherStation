# SimpleWeatherStation

 Simple weather station is based on ESP32. Most of all data are taken from "openweathermap.org" rest is taken from BME280 sensor. All text expressions are in Czech language. You can use your own language but you probably have to adjust text positions to fit correctly on display. Weather station displays around 18 current weather values (like outside temperature, humidity, air pressure, dew point, wind speed and direction, UV index, sunrise/sunset ...) also temperature and humidity indoors via BME280 sensor, plus shows actual weather icon.
 
 # Hardware requirements
 - BME280 sensor (5 volts version) [https://a.aliexpress.com/_msziJ2z]
 
 - ILI9341 TFT LCD display (240x320, 2.4", module) [https://a.aliexpress.com/_mOislNx]
 
 - D1 mini ESP32 [https://a.aliexpress.com/_mPMzAJ3]
 
 # Wiring
  - BME280: 
 
      VIN->VCC, GND->GND, SCL->GPIO22, SDA->GPIO21
 
  - Display: 
   
      GND->GND, VCC->3.3V, CLK->GPIO18, MOSI->GPIO23, RES->GPIO04, DC->GPIO02, MISO->GPIO19, CS->GPIO26
 
  # Software requirements
  Directly from Arduino IDE you can install these libraries: ArduinoJson, Adafruit_BME280 and TFT_eSPI. You will also need "upng" library [https://github.com/elanthis/upng].     Just simply put it to your project folder. 
  
  Then, open Arduino IDE, choose File->Preferences menu and copy Sketchbook location to your file explorer. In it open "libraries" folder and find "TFT_eSPI" folder. Edit "User_Setup.h" file as follows:
  
          // Only define one driver, the other ones must be commented out
          #define ILI9341_DRIVER
          //#define ST7735_DRIVER      // Define additional parameters below for this display
          ...
  
          // ###### EDIT THE PIN NUMBERS IN THE LINES FOLLOWING TO SUIT YOUR ESP32 SETUP   ######

          // For ESP32 Dev board (only tested with ILI9341 display)
          // The hardware SPI can be mapped to any pins
          #define TFT_MISO 19
          #define TFT_MOSI 23
          #define TFT_SCLK 18
          #define TFT_CS   26  // Chip select control pin
          #define TFT_DC    2  // Data Command control pin
          #define TFT_RST   4  // Reset pin (could connect to RST pin)
          //#define TFT_RST  -1  // Set TFT_RST to -1 if display RESET is connected to ESP32 board RST
          ...
  
# Fonts
For this project I've created "Consolas_24" and "PhagsPa_18" fonts with support for Czech diacritic in corresponding *.h files. You can also create your own font suitable for this kind of display (ILI9341), just follow this [https://pages.uoregon.edu/park/Processing/process5.html] tutorial. Then new *.VLW font file convert to HEX format here: [https://tomeko.net/online_tools/file_to_hex.php?lang=en] and put it in header file in your project folder like this:

          #include <pgmspace.h>
          const uint8_t  fontName[] PROGMEM = {
              //Insert byte array here
          };
        
Finally, don't forget to include this file in your project.
  
# Installation
Open this project in Arduino IDE and don't forget to change your board to "ESP32 Dev Module" in Tools menu and also set correct COM port. You will probably need to change Partition scheme to "No OTA (2MB APP/2MB SPIFFS)", then in code change latitude and longitude to your city coordinates. Change "place" variable to name of your city. You will also need API key from [https://openweathermap.org] (don't worry, registration is FREE with limitation of 1000 requests per day). Also don't forget to change your WiFi SSID and password (variables: "ssid" and "password"). Then try to upload code to ESP32. And that's it, enjoy!

![Weather station](https://github.com/eWillyo/SimpleWeatherStation/blob/main/weather_station.jpeg?raw=true)
