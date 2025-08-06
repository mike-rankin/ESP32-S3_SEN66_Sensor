
#include <Arduino.h>
#include <SensirionI2cSen66.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include <ESP32Time.h> 
#include "NotoSansBold15.h"
#include "tinyFont.h"
#include "smallFont.h"
#include "midleFont.h"
#include "bigFont.h"
#include "font18.h"

#ifdef NO_ERROR
#undef NO_ERROR
#endif
#define NO_ERROR 0

static char errorMessage[64];
static int16_t error;

//PWM Backlight 
int Backlight_LedPin = 10;  //Not actually used

//Temperature Min and Max values - MOVED TO GLOBAL SCOPE
float maxTemp = -100.0;  // Initialize to a very low value
float minTemp = 100.0;   // Initialize to a very high value

//SEN55 Sensor
float massConcentrationPm1p0 = 0.0;
float massConcentrationPm2p5 = 0.0;
float massConcentrationPm4p0 = 0.0;
float massConcentrationPm10p0 = 0.0;
float humidity = 0.0;
float temperature = 0.0;
float vocIndex = 0.0;
float noxIndex = 0.0;
uint16_t co2 = 0;


//Adafruit_VEML7700 veml = Adafruit_VEML7700();
SensirionI2cSen66 sensor;
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

//additional variables
int ani = 100;

float maxT=30;
double minT=20;

unsigned long timePased = 0;
int counter=0;

//................colors
#define bck TFT_BLACK
unsigned short grays[13];

float temp = 23.0;
float PPpower[24] = {23.0};    // Initialize with 23°C
float PPpowerT[24] = {23.0};   // Initialize with 23°C
int PPgraph[24] = {6};         // Initialize with middle values

ESP32Time rtc(0);


void setup() {
  Serial.begin(115200);
  Wire.begin(18,17);  //23,22
  //pinMode(15,OUTPUT);
  //digitalWrite(15,1);

  //PWM Backlight
  analogWrite(Backlight_LedPin, 90);

  sensor.begin(Wire, SEN66_I2C_ADDR_6B);

  error = sensor.deviceReset();
    if (error != NO_ERROR) {
        //Serial.print("Error trying to execute deviceReset(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        //Serial.println(errorMessage);
        return;
    }
    delay(1200);
    int8_t serialNumber[32] = {0};
    error = sensor.getSerialNumber(serialNumber, 32);
    if (error != NO_ERROR) {
        //Serial.print("Error trying to execute getSerialNumber(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        //Serial.println(errorMessage);
        return;
    }
    //Serial.print("serialNumber: ");
    //Serial.print((const char*)serialNumber);
    //Serial.println();
    error = sensor.startContinuousMeasurement();
    if (error != NO_ERROR) {
        //Serial.print("Error trying to execute startContinuousMeasurement(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        //Serial.println(errorMessage);
        return;
    }


  rtc.setTime(1609459200);
 // Wire.begin(18, 17);  

  //if (!veml.begin()) {
    //Serial.println("VEML7700 not found");
    //while (1) delay(10);
  //}

  // Podešavanje osjetljivosti (po želji)
 //veml.setGain(VEML7700_GAIN_1_8);  
 //veml.setIntegrationTime(VEML7700_IT_25MS);  

 
  tft.init();
  tft.setRotation(3);

  //sprite.createSprite(320, 170);
  sprite.createSprite(280, 240);

  //set brightness
  //ledcSetup(0, 10000, 8);
  //ledcAttachPin(38, 0);
  //ledcWrite(0, 120);

  // generate 13 levels of gray
  int co = 210;
  for (int i = 0; i < 13; i++) {
    grays[i] = tft.color565(co, co, co);
    co = co - 20;
  }
}


void draw() {

  sprite.fillSprite(TFT_BLACK);
  sprite.drawLine(138, 10, 138, 180, grays[6]);  //138, 10, 138, 270
  sprite.drawLine(10, 108, 134, 108, grays[6]); //10, 108, 134, 108
    sprite.drawLine(10, 180, 270, 180, grays[6]);
  sprite.setTextDatum(0);

  //LEFTSIDE
  sprite.loadFont(midleFont);
  sprite.setTextColor(grays[3], TFT_BLACK);
  sprite.drawString("SEN66", 40, 10);
  sprite.unloadFont();

  sprite.loadFont(font18);
  sprite.setTextColor(grays[6], TFT_BLACK);
  sprite.drawString("RUN:", 6, 110);
  sprite.setTextColor(grays[2], TFT_BLACK);

  // Update min/max temperatures with current reading
  float t = temperature;
  if (t > maxTemp) {
    maxTemp = t;
  }
  if (t < minTemp) {
    minTemp = t;
  }
  
  sprite.setTextColor(grays[3], TFT_BLACK);
  sprite.drawString("TIME", 36, 110);
  sprite.setTextColor(grays[3], TFT_BLACK);
  sprite.unloadFont();

  // draw time without seconds
  sprite.loadFont(tinyFont);
  sprite.setTextColor(grays[4], TFT_BLACK);
  sprite.drawString(rtc.getTime().substring(0, 5), 6, 132);
  sprite.unloadFont();

  // draw some static text
 
  sprite.setTextColor(grays[7], TFT_BLACK);
  sprite.drawString("SECONDS", 92, 157);

  // draw temperature
  sprite.setTextDatum(0);
  sprite.loadFont(bigFont);
  sprite.setTextColor(grays[0], TFT_BLACK);
  sprite.drawFloat(temp, 1, 165, 122); //165, 125
  sprite.drawCircle(268, 122, 5, TFT_WHITE);
  sprite.unloadFont();


  //draw sec rectangle
  sprite.fillRoundRect(90, 132, 42, 22, 2, grays[2]);
  //draw seconds
  sprite.setTextDatum(4);
  sprite.loadFont(font18);
  sprite.setTextColor(TFT_BLACK, grays[2]);
  sprite.drawString(rtc.getTime().substring(6, 8), 111, 144);
  sprite.unloadFont();


  sprite.setTextDatum(0);
  //RIGHT SIDE
  sprite.loadFont(font18);
  sprite.setTextColor(grays[1], TFT_BLACK);
  sprite.drawString("TEMP GRAPH", 144, 10);
  sprite.unloadFont();
  sprite.fillRect(144, 28, 84, 2, grays[10]);
  sprite.setTextColor(grays[3], TFT_BLACK);
  
  sprite.drawString("ESP32-C6 I2C SEN66", 146, 100);
  sprite.fillSmoothRoundRect(144, 34, 174, 60, 3, grays[10], bck);
  sprite.drawLine(170, 39, 170, 88, TFT_WHITE);
  sprite.drawLine(170, 88, 314, 88, TFT_WHITE);

  sprite.setTextDatum(4);

  for (int j = 0; j < 24; j++)
    for (int i = 0; i < PPgraph[j]; i++)
        sprite.fillRect(173 + (j * 6), 83 - (i * 4), 4, 3, TFT_GREEN);


  sprite.setTextColor(grays[2], grays[10]);
  sprite.drawString("MAX", 156, 42);
  sprite.drawString("MIN", 158, 86);

  sprite.loadFont(font18);
  sprite.setTextColor(grays[7], grays[10]);
  sprite.drawString("L", 158, 58);
  sprite.unloadFont();

  // Display min/max temperatures
  sprite.loadFont(font18);
  sprite.setTextColor(grays[2], TFT_BLACK);
  sprite.drawString("MIN: " + String(minTemp, 1), 35, 60);  
  sprite.drawString("MAX: " + String(maxTemp, 1), 35, 80);  
  sprite.unloadFont();

  // Display co2 Level
  //co2_out = co2;
  Serial.println(co2);
  sprite.loadFont(font18);
  sprite.setTextColor(grays[3], TFT_BLACK);
  //sprite.drawString("CO2:", 18, 200);
  char buf[6];
  itoa(co2, buf, 10);
  sprite.drawString("CO2: " + String(buf), 40, 200); 
  sprite.drawString("Humidity: " + String(humidity, 1), 130, 200);
  sprite.drawString("VOC: " + String(vocIndex), 220, 200);
  //sprite.drawString("NOX: " + String(noxIndex), 240, 200);
  sprite.drawString("Pm1p0: " + String(massConcentrationPm1p0,1), 55, 230);  
  sprite.drawString("Pm2p5: " + String(massConcentrationPm2p5,1), 140, 230); 
  sprite.drawString("Pm4p0: " + String(massConcentrationPm4p0,1), 225, 230);
  //sprite.drawString("Pm10p0: " + String(massConcentrationPm10p0), 150, 220);  
  sprite.unloadFont();
  
  sprite.pushSprite(0, 0);
  delay(1000);

  }



void loop() {

 sensor.readMeasuredValues(
 massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
 massConcentrationPm10p0, humidity, temperature, vocIndex, noxIndex,
 co2);

 temp = temperature;

 PPpower[23] = temp;
  for (int i = 23; i > 0; i--)
    PPpower[i - 1] = PPpowerT[i];

  for (int i = 0; i < 24; i++) {
    PPpowerT[i] = PPpower[i];
    if (PPpower[i] < minT) minT = PPpower[i];
    if (PPpower[i] > maxT) maxT = PPpower[i];
  }

  for (int i = 0; i < 24; i++) {
    PPgraph[i] = map(PPpower[i], minT, maxT, 0, 12);
  }
  
  delay(100);
  draw();
}