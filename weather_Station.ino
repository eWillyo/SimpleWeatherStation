// Simple weather station based on ESP32
// Source code by Vilém Pantlík

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "upng.h"
// sensor
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme; // I2C

#define MEASSURE_DELAY 5*60*1000 // should not exceed 1000 measures per day!
#define DISPLAY_DELAY 2*60*1000
#define START_DELAY 5000

// display
#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
#include "Consolas_24.h"
#include "PhagsPa_18.h"

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library
TFT_eSprite weather_img = TFT_eSprite(&tft);

#define IMG_WIDTH 100
#define IMG_HEIGHT 100
#define IMG_COLOR_DEPTH 8

#define BCK_COLOR TFT_OLIVE

// edit this !!!
float latitude = 39.010193;  // your city latitude!
float longitude = 27.122532; // your city longitude!
String place = "your-city-name";
String openWeatherMapAPIKey = "your-open-weather-map-api-key";
// edit this !!!

String serverName = "http://api.openweathermap.org/data/2.5/onecall";
String request_template = "?lat=%f&lon=%f&units=metric&lang=cz&exclude=minutely,hourly,daily&appid=%s";
char request[255];
String icon_url = "http://openweathermap.org/img/wn/%s@2x.png";

upng_t* upng = NULL;
uint16_t icon_bmp[IMG_WIDTH*IMG_HEIGHT];
size_t icon_bmp_size = 0;

// edit this !!!
char ssid[] = "your-wifi-ssid";           //  your network SSID (name)
char password[] = "your-wifi-password";   // your network password
// edit this !!!

float inTemp = 0.0;
float inHum = 0.0;

DynamicJsonDocument weatherJsonDoc(2500);
JsonObject outData;

String w_descript;
String w_icon;
String w_icon_prev;
float outTemp;
float feels_temp;
//float temp_max;
//float temp_min;
float dew_point;
int pressure;
int humidity;
float uvi;
int visibility;
float wind_Speed;
int wind_deg;
int clouds;
int rain_1h=0;
//int rain_3h=0;
int snow_1h=0;
//int snow_3h=0;
String war_event;
String war_desc_split[2];
time_t war_start_utc;
time_t war_stop_utc;
String war_start_text, war_stop_text;
time_t sunrise_utc;
time_t sunset_utc;
String sunset_text, sunrise_text;
int timezone;

TaskHandle_t Task1;

char* directions[] = {"S","SSV","SV","VSV","V","VJV", "JV", "JJV","J","JJZ","JZ","ZJZ","Z","ZSZ","SZ","SSZ"};

// https://stackoverflow.com/questions/9072320/split-string-into-string-array
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

// https://stackoverflow.com/questions/7490660/converting-wind-direction-in-angles-to-text-words
int windAngleToText(int angle)
{
  int val=int((angle/22.5)+.5);
  return val % 16;
}

unsigned char* loadPNG(const unsigned char* data, unsigned long length)
{
  upng = upng_new_from_bytes(data, length);
  if (upng != NULL) 
  {
    upng_decode(upng);
    if (upng_get_error(upng) == UPNG_EOK) 
    {
      Serial.print("Image width: ");
      Serial.println(upng_get_width(upng));
      Serial.print("Image height: ");
      Serial.println(upng_get_height(upng)); 
      Serial.print("BPP: ");
      Serial.println(upng_get_bpp(upng)); 
      if (upng_get_format(upng) == UPNG_RGBA8)
        Serial.println("OK..");
      else {
        Serial.println("Bad format..");
        return NULL;
      }

      Serial.print("Size: ");
      icon_bmp_size = upng_get_size(upng);
      Serial.println(icon_bmp_size);
      return (unsigned char*)upng_get_buffer(upng);
    }
  }

  return NULL;
}

void downloadWeatherIcon(const char* url)
{
  String payload = httpGETRequest(url);
  
  unsigned char* b = loadPNG((unsigned char*)payload.c_str(), payload.length());
  
  if (b == NULL)
  {
    Serial.println("Error while loading PNG!!!");
    return;
  }

  Bitmap2DispClr(b, BCK_COLOR, icon_bmp_size);

  if (upng != NULL)
      upng_free(upng);
}

String httpGETRequest(const char* serverName) {
  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "--"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    delay(2000);
    //ESP.restart();
  }
  // Free resources
  http.end();

  return payload;
}

JsonObject parseJsonDoc(String doc)
{
  JsonObject obj;
  
  auto error = deserializeJson(weatherJsonDoc, doc.c_str());
  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return obj;
  }

  return obj = weatherJsonDoc.as<JsonObject>();
}

void dispFnc(void* parameter)
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.loadFont(Consolas_24);

  tft.setCursor(65,120);
  tft.println("Spouštím se..");
  
  delay(START_DELAY);
  while(true)
  {
    // first, check wifi..
    if (WiFi.status() != WL_CONNECTED) { 
      Serial.println("WiFi Error!");
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_WHITE);
      tft.loadFont(Consolas_24);
      tft.setCursor(65,120);
      tft.println("Chyba WiFi..");
      delay(5000);
      ESP.restart();
    }
    // display data..
    tft.fillScreen(BCK_COLOR);
    // layout
    tft.fillRoundRect(115, 5, 200, 165, 3, TFT_DARKGREY); // main
    tft.fillRoundRect(165, 175, 145, 85, 3, TFT_DARKGREY); // outdoor
    tft.fillRoundRect(10, 175, 145, 85, 3, TFT_DARKGREY); // indoor
    //font
    tft.setTextColor(TFT_BLACK);
    tft.loadFont(Consolas_24);
    tft.setCursor(10,10);
    // place
    String main = place + String(",");
    tft.println(main);
    // temp. in
    tft.setTextColor(TFT_RED);
    tft.setCursor(40,200);
    main = String(inTemp, 1) + " °C";
    tft.println(main);
    // hum. in
    tft.setTextColor(TFT_NAVY);
    tft.setCursor(40,220);
    main = String(inHum, 0) + " %";
    tft.println(main);
    // temp. out
    tft.setTextColor(TFT_RED);
    tft.setCursor(195,200);
    main = String(outTemp, 1) + " °C";
    tft.println(main);
    // hum. out
    tft.setTextColor(TFT_NAVY);
    tft.setCursor(195,220);
    main = String(humidity) + " %";
    tft.println(main);
    tft.unloadFont();
    // main
    tft.setTextColor(TFT_WHITE);
    tft.loadFont(PhagsPa_18);
    // temp
    // feels
    tft.setCursor(120,10);
    main = String("Pocitově: ") + String(feels_temp, 1) + " °C";
    tft.println(main);
    /*// temp min/max
    tft.setCursor(125,30);
    main = String("Min/Max: ") + String(temp_min, 1) + String("/") + String(temp_max, 1) + " °C";
    tft.println(main);*/
    // dew point
    tft.setCursor(120,30);
    main = String("Rosný bod: ") + String(dew_point, 1) + " °C";
    tft.println(main);
    // pressure
    tft.setCursor(120,50);
    main = String("Tlak: ") + String(pressure) + String( " hPa   UVI: ") + String(uvi,1);
    tft.println(main);
    // wind
    tft.setCursor(120,70);
    main = String("Vítr: ") + String(directions[windAngleToText(wind_deg)]) + String(", ") + String(wind_Speed,1) + String(" m/s");
    tft.println(main);
    // clouds
    tft.setCursor(120,90);
    main = String("Oblaka: ") + String(clouds) + String(" %");
    tft.println(main);
    // rain/snow
    tft.setCursor(120,110);
    if (war_event == String("null")) // normal info
    {
      if (snow_1h == 0) { // no snow
        main = String("Déšť: ") + String(rain_1h) + String(" 1h/mm");
      }
      else {
        main = String("Sníh: ") + String(snow_1h) + String(" 1h/mm");
      }
      tft.println(main);
      // sunrise/sunset
      tft.setCursor(120,130);
      main = String("Slunce: V ") + sunrise_text + String(" Z ") + sunset_text;
      tft.println(main);
      // visibility !!!
      tft.setCursor(120,150);
      main = String("Viditelnost: ") + visibility + String(" m");
      tft.println(main);
    }
    else { // warning
      tft.setTextColor(TFT_RED);
      tft.setCursor(120,110);
      main = war_event + String(" (") + war_start_text + String(" - ") + war_stop_text + String(")");
      tft.println(main);
      // warning description
      tft.setTextColor(TFT_WHITE);
      // line 1
      tft.setCursor(120,130);
      main = war_desc_split[0];
      tft.println(main);
      // line 2
      tft.setCursor(120,150);
      main = war_desc_split[1];
      tft.println(main);
    }
    // weather descr.
    main = w_descript;
    tft.setTextColor(TFT_BLACK);
    tft.setCursor(10,35);
    tft.println(main);
    //descr. in
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(15,180);
    tft.println("Vevnitř");
    //descr. out
    tft.setCursor(170,180);
    tft.println("Venku");
    tft.unloadFont();
    // weather icon
    drawBMP(icon_bmp, 10, 60, IMG_WIDTH, (IMG_HEIGHT-20));
    
    delay(DISPLAY_DELAY);
  }
}

void Bitmap2DispClr(unsigned char* data, uint16_t bckColor, size_t length)
{
  uint16_t* converted = icon_bmp;
  int16_t ptr = 0;

  if (data == NULL)
    return;

  memset(converted, 0, (IMG_WIDTH * IMG_HEIGHT * sizeof(uint16_t)));

  for (int16_t i = 0; i < length;)
  {
    unsigned char r = data[i++];
    unsigned char g = data[i++];
    unsigned char b = data[i++];
   
    if (data[i++] < 16) {// alpha  
      converted[ptr++] = bckColor;
    }
    else {
      converted[ptr++] = tft.color565(r,g,b);
    }
  }
}

void drawBMP(uint16_t *data, int16_t x, int16_t y, int16_t w, int16_t h) 
{
  if ((x >= tft.width()) || (y >= tft.height())) return;

  tft.pushImage(x, y, w, h, data);
}

void setup() {
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.invertDisplay(true);
  tft.setSwapBytes(true);
  
  Serial.begin(115200);
  Serial.println("Attempting to connect to WPA network...");
  Serial.print("SSID: ");
  Serial.println(ssid);

  int count = 0;

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) { 
    delay(400);
    Serial.print(".");
    count++;

    if(count > 10)
    {
      Serial.println("Could not connect to WiFi!");
      tft.setTextColor(TFT_WHITE);
      tft.loadFont(Consolas_24);

      tft.setCursor(65,120);
      tft.println("Chyba WiFi..");
      delay(5000);
      ESP.restart();
    }
  }
  
  Serial.println("Connected to wifi");
  
  // sensor
  bool status = bme.begin(0x76);  
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    tft.setTextColor(TFT_WHITE);
    tft.loadFont(Consolas_24);

    tft.setCursor(65,120);
    tft.println("Chyba čidla..");
    delay(5000);
    ESP.restart();
  }

  // create request..
  sprintf(request, request_template.c_str(), latitude, longitude, openWeatherMapAPIKey.c_str()); 

  // start display thread
  xTaskCreatePinnedToCore(
      dispFnc, /* Function to implement the task */
      "display", /* Name of the task */
      40000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      1,  /* Priority of the task */
      &Task1,  /* Task handle. */
      1); /* Core where the task should run */
}

void loop() 
{
  // inside
  inTemp = bme.readTemperature();
  inHum = bme.readHumidity();
  pressure = int(bme.readPressure() / 100.0F);
    
  if (isnan(inTemp) || isnan(inHum) || isnan(pressure)) 
  {
    Serial.println("Sensor reading error!");
    //delay(5000);
    //ESP.restart();
  }
  
  // outside
  String serverPath = serverName + request;
      
  String payload = httpGETRequest(serverPath.c_str());
  outData = parseJsonDoc(payload);

  rain_1h = snow_1h = 0;
  war_event = String("");

  w_descript = outData["current"]["weather"][0]["description"].as<String>();
  w_icon = outData["current"]["weather"][0]["icon"].as<String>();
  outTemp = outData["current"]["temp"].as<float>();
  feels_temp = outData["current"]["feels_like"].as<float>();
  dew_point = outData["current"]["dew_point"].as<float>();
  humidity = outData["current"]["humidity"].as<int>();
  uvi = outData["current"]["uvi"].as<float>();
  visibility = outData["current"]["visibility"].as<int>();
  wind_Speed = outData["current"]["wind_speed"].as<float>();
  wind_deg = outData["current"]["wind_deg"].as<int>();
  clouds = outData["current"]["clouds"].as<int>();
  rain_1h = outData["current"]["rain"]["1h"].as<int>();
  snow_1h = outData["current"]["snow"]["1h"].as<int>();
  sunrise_utc = outData["current"]["sunrise"].as<time_t>();
  sunset_utc = outData["current"]["sunset"].as<time_t>();
  timezone = outData["timezone_offset"].as<int>();
  sunrise_utc += timezone;
  sunset_utc += timezone;
  war_event = outData["alerts"]["event"].as<String>();
  String war_desc = outData["alerts"]["description"].as<String>();
  war_start_utc = outData["alerts"]["start"].as<time_t>();
  war_stop_utc = outData["alerts"]["stop"].as<time_t>();

  war_desc_split[0] = getValue(war_desc, '\n', 0);
  war_desc_split[1] = getValue(war_desc, '\n', 1);

  struct tm  ts;
  char buf[80];
  ts = *localtime(&sunrise_utc);
  strftime(buf, sizeof(buf), "%H:%M", &ts);
  sunrise_text = String(buf);
  ts = *localtime(&sunset_utc);
  strftime(buf, sizeof(buf), "%H:%M", &ts);
  sunset_text = String(buf);

  ts = *localtime(&war_start_utc);
  strftime(buf, sizeof(buf), "%H:%M", &ts);
  war_start_text = String(buf);
  ts = *localtime(&war_stop_utc);
  strftime(buf, sizeof(buf), "%H:%M", &ts);
  war_stop_text = String(buf);
  
  // icon
  char buff[100];

  if (w_icon != w_icon_prev)
  {
    w_icon_prev = w_icon;
      
    sprintf(buff, icon_url.c_str(), w_icon.c_str());
    downloadWeatherIcon(buff);
  }
    
  delay(MEASSURE_DELAY);
}
