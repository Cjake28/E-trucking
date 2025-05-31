#include "GPSModule.h"

GPSModule::GPSModule(HardwareSerial &serial, int rx, int tx, uint32_t baudRate)
  : gpsSerial(serial), rxPin(rx), txPin(tx), baud(baudRate) {}

void GPSModule::begin() {
  gpsSerial.begin(baud, SERIAL_8N1, rxPin, txPin);
}

bool GPSModule::available() {
  return gpsSerial.available() > 0;
}

void GPSModule::feed() {
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }
}

bool GPSModule::locationValid() {
  return gps.location.isValid();
}

bool GPSModule::locationUpdated() {
  return gps.location.isUpdated();
}

double GPSModule::latitude() {
  return gps.location.lat();
}

double GPSModule::longitude() {
  return gps.location.lng();
}

float GPSModule::hdop() {
  return gps.hdop.hdop();
}

uint32_t GPSModule::satellites() {
  return gps.satellites.value();
}

float GPSModule::speed(){
  return gps.speed.kmph();
}

GPSData GPSModule::getData(int utcOffsetHours) {
  GPSData data = {};
  data.latitude = gps.location.lat();
  data.longitude = gps.location.lng();
  data.speedKmph = gps.speed.kmph();

  TinyGPSDate &d = gps.date;
  TinyGPSTime &t = gps.time;

  if (d.isValid() && t.isValid()) {
    int year = d.year();
    int month = d.month();
    int day = d.day();
    int hour = t.hour() + utcOffsetHours;
    int minute = t.minute();
    int second = t.second();

    if (hour >= 24) {
      hour -= 24;
      day++;
    }
    data.year = year;
    data.month = month;
    data.day = day;
    data.hour = hour;
    data.minute = minute;
    data.second = second;
  }
  return data;
}

GPSStringData GPSModule::getStringData(int utcOffsetHours) {
  GPSStringData data;

  data.latitude = gps.location.isValid() ? String(gps.location.lat(), 6) : "Invalid";
  data.longitude = gps.location.isValid() ? String(gps.location.lng(), 6) : "Invalid";
  data.speedKmph = gps.speed.isValid() ? String(gps.speed.kmph(), 2) : "Invalid";

  TinyGPSDate &d = gps.date;
  TinyGPSTime &t = gps.time;

  if (d.isValid() && t.isValid()) {
    int hour = t.hour() + utcOffsetHours;
    int day = d.day();

    if (hour >= 24) {
      hour -= 24;
      day += 1; // crude, no month rollover
    }

    data.year = String(d.year());
    data.month = String(d.month());
    data.day = String(day);  // possibly incremented
    data.hour = String(hour);
    data.minute = String(t.minute());
    data.second = String(t.second());
  } else {
    data.year = data.month = data.day = "Invalid";
    data.hour = data.minute = data.second = "Invalid";
  }

  return data;
}