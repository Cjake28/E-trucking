#include "utilities.h"
#include "gpsUtils.h"
#include "Gpsmodule.h"
#include <Arduino.h>

#define ENABLE_USER_CONFIG
#define ENABLE_USER_AUTH
#define ENABLE_DATABASE
#define ENABLE_GSM_NETWORK
#define ENABLE_ESP_SSLCLIENT

#define NETWORK_APN "internet"

String Truck_ID  = "truck_12345";
float speedLimit = 70;
bool globOverSpeed = false;
bool active = false;
bool activeSet = false;
bool gpsFixStatus = false;

const unsigned long DB_UPDATE_INTERVAL   = 45UL * 1000;  
const unsigned long OVERSPEED_CHECK_INTERVAL = 400;     
const unsigned long OVERSPEED_UPDATE_INTERVAL = 1000; 

unsigned long  lastDbUpdate   = 0;
unsigned long  lastOverspeedCheck = 0;
unsigned long  lastOverspeedUpdate = 0;

const String path = "/Trucks/" + Truck_ID +"/data/";
const String activePath = path + "active";

#define SerialMon Serial
#define SerialAT Serial1

#define GSM_PIN ""
const char apn[] = "internet";
const char gprsUser[] = "";
const char gprsPass[] = "";

#define UART_BAUD 115200

#include <TinyGsmClient.h>
#include <FirebaseClient.h>
#include "ExampleFunctions.h"

#define API_KEY "AIzaSyDWV7_xKJ9vRvtBceQdTb5akTSx3jKk1Bw"
#define USER_EMAIL "truck123@gmail.com"
#define USER_PASSWORD "123456"
#define DATABASE_URL "https://e-trucking-8d905-default-rtdb.asia-southeast1.firebasedatabase.app"

GPSModule gps(Serial2, GPS_RX_PIN, GPS_TX_PIN, GPS_BAUDRATE);

TinyGsm modem(SerialAT);

TinyGsmClient gsm_client(modem, 0), stream_gsm_client(modem, 1);

ESP_SSLClient ssl_client, stream_ssl_client;

GSMNetwork gsm_network(&modem, GSM_PIN, apn, gprsUser, gprsPass);

using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client, getNetwork(gsm_network)), streamClient(stream_ssl_client, getNetwork(gsm_network));

UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD);

FirebaseApp app;
RealtimeDatabase Database;
AsyncResult streamResult;

void SendPeriodicData();
void checkAndAlertOverspeed();
void updateRealtimeDB(bool overSpeed);
void get_active_data(AsyncResult &aResult);
void processData(AsyncResult &aResult);
void update_GPS_Fix();

void setup(){
	Serial.begin(115200);

    // Resetting the modem
#ifdef BOARD_POWERON_PIN
	pinMode(BOARD_POWERON_PIN, OUTPUT);
	digitalWrite(BOARD_POWERON_PIN, HIGH);
#endif

    // Set modem reset pin ,reset modem
#ifdef MODEM_RESET_PIN
	pinMode(MODEM_RESET_PIN, OUTPUT);
	digitalWrite(MODEM_RESET_PIN, !MODEM_RESET_LEVEL); delay(100);
	digitalWrite(MODEM_RESET_PIN, MODEM_RESET_LEVEL); delay(2600);
	digitalWrite(MODEM_RESET_PIN, !MODEM_RESET_LEVEL);
#endif

	pinMode(BOARD_PWRKEY_PIN, OUTPUT);
	digitalWrite(BOARD_PWRKEY_PIN, LOW);
  delay(100);
	digitalWrite(BOARD_PWRKEY_PIN, HIGH);
	delay(100);
	digitalWrite(BOARD_PWRKEY_PIN, LOW);

	Serial.println("Start modem...");
	delay(2000);

	gps.begin();

	SerialAT.begin(UART_BAUD, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);

	int retry = 0;
	while (!modem.testAT(1000)) {
			Serial.println(".");
			if (retry++ > 10) {
					digitalWrite(BOARD_PWRKEY_PIN, LOW);
					delay(100);
					digitalWrite(BOARD_PWRKEY_PIN, HIGH);
					delay(1000);
					digitalWrite(BOARD_PWRKEY_PIN, LOW);
					retry = 0;
			}
	}
	Serial.println();

	SimStatus sim = SIM_ERROR;
	while (sim != SIM_READY) {
		sim = modem.getSimStatus();
		switch (sim) {
		case SIM_READY:
			Serial.println("SIM card online");
			break;
		case SIM_LOCKED:
			Serial.println("The SIM card is locked. Please unlock the SIM card first.");
			break;
		default:
			break;
		}
		delay(400);
	}


	if (!modem.setNetworkMode(38)) {
		Serial.println("Set network mode failed!");
	}
	String mode = modem.getNetworkModes();
	Serial.print("Current network mode : ");
	Serial.println(mode);

#ifdef NETWORK_APN
	Serial.printf("Set network apn : %s\n", NETWORK_APN);
	modem.sendAT(GF("+CGDCONT=1,\"IP\",\""), NETWORK_APN, "\"");
	if (modem.waitResponse() != 1) {
		Serial.println("Set network apn error !");
	}
#endif

if (!modem.isGprsConnected()) {
	Serial.println("GPRS not connected!");
} else {
	Serial.println("GPRS connected!");
}
	Serial.println("Modem connected to GPRS");
	String name = modem.getModemName();
	DBG("Modem Name:", name);

	String modemInfo = modem.getModemInfo();
	DBG("Modem Info:", modemInfo);
    
	Serial.print("IP Address: ");
	Serial.println(modem.localIP());

	Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);

	ssl_client.setInsecure();
	ssl_client.setDebugLevel(1);
	ssl_client.setBufferSizes(4096, 2048);
	ssl_client.setClient(&gsm_client);

	stream_ssl_client.setInsecure();
	stream_ssl_client.setDebugLevel(1);
	stream_ssl_client.setBufferSizes(4096 , 2048 );
	stream_ssl_client.setClient(&stream_gsm_client);

	Serial.println("Initializing app...");
	initializeApp(aClient, app, getAuth(user_auth), auth_debug_print, "🔐 authTask");

	app.getApp<RealtimeDatabase>(Database);

	Database.url(DATABASE_URL);

	streamClient.setSSEFilters("get,put,patch,keep-alive,cancel,auth_revoked");

	Database.get(streamClient, activePath, get_active_data, true /* SSE mode */, "streamTask");

}

unsigned long ms = 0;

void loop(){

  gps.feed();

  app.loop();

  Database.loop();

	if(activeSet && active){
		SendPeriodicData();
		checkAndAlertOverspeed();
	}

  delay(50);
}

void SendPeriodicData() {
  if (!app.ready()) return;
  
  unsigned long now = millis();
  if (now - lastDbUpdate < DB_UPDATE_INTERVAL) return;
  lastDbUpdate = now;

  if (!gps.locationValid() || !gps.locationUpdated()) {
    Serial.println("Periodic update: no valid GPS fix.");
		gpsFixStatus = false;
		delay(20);
		update_GPS_Fix(gpsFixStatus);
		return;
  }
	
	if(!gpsFixStatus){
		gpsFixStatus = true;
		delay(20);
		update_GPS_Fix(gpsFixStatus);
	}
  bool overSpeed = gps.speed() > speedLimit;
  GPSStringData d = gps.getStringData();

  Serial.println("Periodic GPS Update:");
  Serial.printf("  Lat: %s\n  Lon: %s\n  Spd: %s km/h\n  %s %s-%s-%s\n",
                d.latitude.c_str(),
                d.longitude.c_str(),
                d.speedKmph.c_str(),
                d.hour.c_str(), d.year.c_str(), d.month.c_str(), d.day.c_str());

	updateRealtimeDB(overSpeed);
}

void checkAndAlertOverspeed(){
  unsigned long now = millis();
  if (now - lastOverspeedCheck < OVERSPEED_CHECK_INTERVAL) return;
  lastOverspeedCheck = now;

  if (!gps.locationValid() || !gps.locationUpdated()) return;

  float currentSpeed = gps.speed();

  if (currentSpeed <= speedLimit){
		if (globOverSpeed && (now - lastOverspeedUpdate >= OVERSPEED_UPDATE_INTERVAL)) {
			globOverSpeed = false;
			updateRealtimeDB(false);
			Serial.println("Overspeed alert cleared.");
		}

	} else {
		if (!globOverSpeed) {
			globOverSpeed = true;
			lastOverspeedUpdate = now;
			updateRealtimeDB(true);
			Serial.println("Overspeed alert sent.");
		}
	}
};

void updateRealtimeDB(bool overSpeedState) {
  GPSStringData d = gps.getStringData();  

  JsonWriter writer;
  object_t json, o1, o2, o3, o4;
  writer.create(o1, "lat",       d.latitude);
  writer.create(o2, "lon",       d.longitude);
  writer.create(o3, "speed",     d.speedKmph);
  writer.create(o4, "overSpeed", overSpeedState);
  writer.join(json, 4, o1, o2, o3, o4);

  Database.update<object_t>(
    aClient,
    path,
    json,
    processData,
    "updateTask"
  );
}

void update_GPS_Fix(bool status){
	JsonWriter writer;
	object_t json, o1;
	writer.create(o1, "gpsFix", status);
	writer.join(json, 1, o1);
	Database.update<object_t>(
		aClient,
		path,
		json,
		processData,
		"updateGPSFixTask"
	);
}

void get_active_data(AsyncResult &aResult){
	if (!aResult.isResult())
		return;

	if (aResult.isEvent()){
		Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.eventLog().message().c_str(), aResult.eventLog().code());
	}

	if (aResult.isDebug()){
		Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
	}

	if (aResult.isError()){
		Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
		return;
	}

	if (aResult.available()){
		RealtimeDatabaseResult &RTDB = aResult.to<RealtimeDatabaseResult>();
		if (RTDB.isStream()){

			Serial.println("----------------------------");
			Firebase.printf("task: %s\n", aResult.uid().c_str());
			Firebase.printf("event: %s\n", RTDB.event().c_str());
			Firebase.printf("path: %s\n", RTDB.dataPath().c_str());
			Firebase.printf("data: %s\n", RTDB.to<const char *>());
			Firebase.printf("type: %d\n", RTDB.type());

			String eventType = RTDB.event();      // “put”, “patch”, “keep-alive”, etc.
			String dataPath  = RTDB.dataPath();   // e.g. “/” or “/someChild”
			String payload   = RTDB.to<String>(); // e.g. "true", "false", or “null”

			if (!activeSet && eventType == "put" && dataPath == "/") {
    // The payload is the entire node’s value (a JSON primitive true/false)
				active = (payload == "true");
				activeSet = true;
				Serial.printf("[SSE] Initial flag = %s\n", active ? "true" : "false");
				return;
			}

			if (activeSet && (eventType == "put" || eventType == "patch") && dataPath == "/") {
				bool newVal = (payload == "true");
				if (newVal != active) {
					active = newVal;
					Serial.printf("[SSE] Updated flag = %s\n", active ? "true" : "false");
				}
				return;
			}
		}
		else{
			Serial.println("----------------------------");
			Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
		}

		Firebase.printf("Free Heap: %d\n", ESP.getFreeHeap());
	}
}

void processData(AsyncResult &aResult){
	if (!aResult.isResult())
			return;

	if (aResult.isEvent()){
		Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.eventLog().message().c_str(), aResult.eventLog().code());
	}

	if (aResult.isDebug()){
		Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
	}

	if (aResult.isError()){
		Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
	}

	if (aResult.available()){
		RealtimeDatabaseResult &RTDB = aResult.to<RealtimeDatabaseResult>();
		if (RTDB.isStream()){
			Serial.println("----------------------------");
			Firebase.printf("task: %s\n", aResult.uid().c_str());
			Firebase.printf("event: %s\n", RTDB.event().c_str());
			Firebase.printf("path: %s\n", RTDB.dataPath().c_str());
			Firebase.printf("data: %s\n", RTDB.to<const char *>());
			Firebase.printf("type: %d\n", RTDB.type());

			// The stream event from RealtimeDatabaseResult can be converted to the values as following.
			bool v1 = RTDB.to<bool>();
			int v2 = RTDB.to<int>();
			float v3 = RTDB.to<float>();
			double v4 = RTDB.to<double>();
			String v5 = RTDB.to<String>();
		}
		else{
			Serial.println("----------------------------");
			Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
		}

		Firebase.printf("Free Heap: %d\n", ESP.getFreeHeap());
	}
}