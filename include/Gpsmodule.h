#ifndef GPSMODULE_H
#define GPSMODULE_H

#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

struct GPSData {
  double latitude;
  double longitude;
  float speedKmph;
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
};

struct GPSStringData {
  String latitude;
  String longitude;
  String speedKmph;
  String year;
  String month;
  String day;
  String hour;
  String minute;
  String second;
};

class GPSModule {
public:
  GPSModule(HardwareSerial &serial, int rxPin, int txPin, uint32_t baud);
  void    begin();
  bool    available();
  void    feed();
  bool    locationValid();
  bool    locationUpdated();
  double  latitude();
  double  longitude();
  float   hdop();
  uint32_t satellites();
  float   speed();

  GPSData getData(int utcOffsetHours = 8);

  GPSStringData getStringData(int utcOffsetHours = 8);

private:
  HardwareSerial &gpsSerial;
  int rxPin;
  int txPin;
  uint32_t baud;
  TinyGPSPlus gps;
};

#endif 