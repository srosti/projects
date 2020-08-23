//#include <Arduino.h>
//
//void setup() {
//  // put your setup code here, to run once:
//  Serial.begin(115200);
//}
//
//void loop() {
//  // put your main code here, to run repeatedly:
//  Serial.println("Hello World");
//  delay(1000);
//}

/*************************************************** 
  This is a library for the Adafruit PT100/P1000 RTD Sensor w/MAX31865

  Designed specifically to work with the Adafruit RTD Sensor
  ----> https://www.adafruit.com/products/3328

  This sensor uses SPI to communicate, 4 pins are required to  
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MAX31865.h>
#include <M5Stack.h>

// Use software SPI: CS, DI, DO, CLK
// Note: Software SPI does not work on esp32 - srosti
//Adafruit_MAX31865 thermo = Adafruit_MAX31865(5, 23, 19, 18);
// Use hardware SPI, just pass in the CS pin
Adafruit_MAX31865 thermo = Adafruit_MAX31865(5);

// The value of the Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000
#define RREF      430.0
// The 'nominal' 0-degrees-C resistance of the sensor
// 100.0 for PT100, 1000.0 for PT1000
#define RNOMINAL  100.0

// Mash temperatures typically start around 150 degrees (F)
int target_temp = 150;

float current_temp = 0;


void setup() {
  Serial.begin(115200);
  Serial.println("Adafruit MAX31865 PT100 Sensor Test!");
  
  thermo.begin(MAX31865_3WIRE);  // set to 2/3/4 wire as necessary

  M5.begin();
  M5.Power.begin();
  M5.lcd.fillScreen(WHITE);
  M5.Lcd.setCursor(10,10);
  M5.Lcd.setTextColor(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.printf("RIMS Brew Controller");

}

void display_temp(float temp) {
  // create a white square to "erase" any previous temperature
  // already on the screen
  M5.Lcd.fillRect(100, 50, 300, 50, WHITE);
  // display the temp
  M5.Lcd.setTextColor(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0,50);
  M5.Lcd.printf("  Current: % 6.2f (F)\n", (temp*1.8)+32);
  M5.Lcd.printf("  Target:  %d.00 (F)\n", target_temp);
}

void get_keypress() {
  if (M5.BtnA.wasPressed()) {
//    target_temp--;
  } else if (M5.BtnB.wasPressed()) {
    // Nothing defined yet for middle button
  } else if (M5.BtnC.wasPressed()) {
    target_temp++;
  } 

}

void get_temp() {
  thermo.begin(MAX31865_3WIRE);  // set to 2WIRE or 4WIRE as necessary
  uint16_t rtd = thermo.readRTD();

  float ratio = rtd;
  ratio /= 32768;
  current_temp = thermo.temperature(RNOMINAL, RREF);
//  Serial.print("RTD value: "); Serial.println(rtd);
//  Serial.print("Ratio = "); Serial.println(ratio,8);
//  Serial.print("Resistance = "); Serial.println(RREF*ratio,8);
//  Serial.print("Temperature = "); Serial.println(thermo.temperature(RNOMINAL, RREF));

  // Check and print any faults
  uint8_t fault = thermo.readFault();
  if (fault) {
    Serial.print("Fault 0x"); Serial.println(fault, HEX);
    if (fault & MAX31865_FAULT_HIGHTHRESH) {
      Serial.println("RTD High Threshold"); 
    }
    if (fault & MAX31865_FAULT_LOWTHRESH) {
  } else if (M5.BtnB.wasPressed()) {
      Serial.println("REFIN- > 0.85 x Bias"); 
    }
    if (fault & MAX31865_FAULT_REFINHIGH) {
      Serial.println("REFIN- < 0.85 x Bias - FORCE- open"); 
    }
    if (fault & MAX31865_FAULT_RTDINLOW) {
      Serial.println("RTDIN- < 0.85 x Bias - FORCE- open"); 
    }
    if (fault & MAX31865_FAULT_OVUV) {
      Serial.println("Under/Over voltage"); 
    }
    thermo.clearFault();
  }

}

void loop() {

  M5.update();

  get_keypress();

  get_temp();

  display_temp(current_temp);
}
