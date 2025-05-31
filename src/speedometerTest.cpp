// #include "gpsUtils.h"
// #include "Gpsmodule.h"
// #include <Wire.h>
// #include <LiquidCrystal_I2C.h>

// LiquidCrystal_I2C lcd(0x27, 16, 2);

// GPSModule gps(Serial2, GPS_RX_PIN, GPS_TX_PIN, GPS_BAUDRATE);

// void setup() {

//   Serial.begin(115200);
//   gps.begin();
//   Wire.begin(21, 22);
  
//   lcd.init();
  
//   lcd.backlight();
  
//   lcd.clear();
//   lcd.setCursor(0, 0);
//   lcd.print("Loading ->");
  

//   for (int pos = 0; pos < 16; pos++) {
//     lcd.setCursor(pos, 1);
//     lcd.print("*");
//     delay(200);
//     lcd.setCursor(pos, 1);
//     lcd.print(" ");
//   }
//   delay(1000);
// }

// void loop() {

//   gps.feed(); 

//   if (gps.locationValid()) {
// 		if (gps.locationUpdated()){
// 			GPSStringData gpsData = gps.getStringData();
// 				lcd.clear();
// 				lcd.setCursor(6, 0);
// 				lcd.print("Speed:");
// 				lcd.setCursor(5, 1);
// 				lcd.print(gpsData.speedKmph);
// 				lcd.print(" km/h");
//     }
// 	}
//   delay(50);
// }
