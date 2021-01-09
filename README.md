# SimpleWeatherStation
 Simple weather station based on ESP32. Most of all data are taken from "openweathermap.org" rest is taken from BME280 sensor. All text expressions are in Czech language. You can use your own language but you probably have to adjust text positions to fit correctly on display.
 
 # Hardware requirements
 BME280 sensor (5 volt version) [https://a.aliexpress.com/_msziJ2z]
 
 ILI9341 TFT LCD display (240x320, 2.4", module) [https://a.aliexpress.com/_mOislNx]
 
 D1 mini ESP32 [https://a.aliexpress.com/_mPMzAJ3]
 
 # Wiring
 BME280: VIN->VCC, GND->GND, SCL->GPIO22, SDA->GPIO21
 
 Display: GND->GND, VCC->3.3V, CLK->GPIO18, MOSI->GPIO23, RES->GPIO04, DC->GPIO02, MISO->GPIO19, CS->GPIO26
