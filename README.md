# SimpleWeatherStation
 Simple weather station based on ESP32. Most of all data are taken from "openweathermap.org" rest is taken from BME280 sensor. All text expressions are in Czech language. You can use your own language but you probably have to adjust text positions to fit correctly on display.
 
 # Hardware requirements
 BME280 sensor (5 volt version) [https://a.aliexpress.com/_msziJ2z]
 
 ILI9341 TFT LCD display (240x320, 2.4", module) [https://a.aliexpress.com/_mOislNx]
 
 D1 mini ESP32 [https://a.aliexpress.com/_mPMzAJ3]
 
 # Wiring
 BME280: VIN->VCC, GND->GND, SCL->GPIO22, SDA->GPIO21
 
 Display: GND->GND, VCC->3.3V, CLK->GPIO18, MOSI->GPIO23, RES->GPIO04, DC->GPIO02, MISO->GPIO19, CS->GPIO26
 
  # Software requirements
  Directly from Arduino IDE you can install these library: ArduinoJson, Adafruit_BME280 and TFT_eSPI. You will also need "upng" library [https://github.com/elanthis/upng].     Just simply put it to your project folder. 
  
  Then, open Arduino IDE, File->properties and copy project location to your file explorer. In it open "libraries" folder and find   "TFT_eSPI" folder. In it edit "User_Setup.h" as follows:
  
  *********************************
  
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
  
  *********************************
  
# Installation
Open this project in Arduion IDE and don't forget to change your board to "ESP32 Dev Module" and also set correct COM port. You will probably need to change Partition scheme to "No OTA (2MB APP/2MB SPIFFS)", then try to upload code to ESP32. 
