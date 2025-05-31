// #include "utilities.h"
// #include "gpsUtils.h"
// #include "Gpsmodule.h"

// #define ENABLE_USER_CONFIG
// #define ENABLE_USER_AUTH
// #define ENABLE_DATABASE
// #define ENABLE_GSM_NETWORK
// #define ENABLE_ESP_SSLCLIENT

// #define NETWORK_APN "internet"

// #define Truck_ID "truck_12345"
// float speedLimit = 70;
// unsigned long ms = 0;
// unsigned long gp = 0;
// bool overSpeed = false;

// // Set serial for debug console (to the Serial Monitor, default speed 115200)
// #define SerialMon Serial

// // Set serial for AT commands (to the module)
// // Use Hardware Serial on Mega, Leonardo, Micro
// #define SerialAT Serial1

// // set GSM PIN, if any
// #define GSM_PIN ""

// // Your GPRS credentials, if any
// const char apn[] = "internet";
// const char gprsUser[] = "";
// const char gprsPass[] = "";

// #define UART_BAUD 115200

// #include <Arduino.h>
// // Include TinyGsmClient.h first and followed by FirebaseClient.h
// #include <TinyGsmClient.h>
// #include <FirebaseClient.h>
// #include "ExampleFunctions.h" // Provides the functions used in the examples.


// #define API_KEY "AIzaSyCFVW2rQELdRrZjb5fV0m0iue5e53cAvv0"
// #define USER_EMAIL "supnetcjs@gmail.com"
// #define USER_PASSWORD "123456"
// #define DATABASE_URL "https://gpstracking-a0007-default-rtdb.asia-southeast1.firebasedatabase.app"

// 	GPSModule gps(Serial2, GPS_RX_PIN, GPS_TX_PIN, GPS_BAUDRATE);

// TinyGsm modem(SerialAT);

// TinyGsmClient gsm_client(modem, 0), stream_gsm_client(modem, 1);

// // The ESP_SSLClient uses PSRAM by default (if it is available), for PSRAM usage, see https://github.com/mobizt/FirebaseClient#memory-options
// // For ESP_SSLClient documentation, see https://github.com/mobizt/ESP_SSLClient
// ESP_SSLClient ssl_client, stream_ssl_client;

// GSMNetwork gsm_network(&modem, GSM_PIN, apn, gprsUser, gprsPass);

// using AsyncClient = AsyncClientClass;
// AsyncClient aClient(ssl_client, getNetwork(gsm_network)), streamClient(stream_ssl_client, getNetwork(gsm_network));

// UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD);

// FirebaseApp app;
// RealtimeDatabase Database;
// AsyncResult streamResult;

// void processData(AsyncResult &aResult);

// void setup(){
// 	Serial.begin(115200);

//     // Resetting the modem
// #ifdef BOARD_POWERON_PIN
// 	pinMode(BOARD_POWERON_PIN, OUTPUT);
// 	digitalWrite(BOARD_POWERON_PIN, HIGH);
// #endif

//     // Set modem reset pin ,reset modem
// #ifdef MODEM_RESET_PIN
// 	pinMode(MODEM_RESET_PIN, OUTPUT);
// 	digitalWrite(MODEM_RESET_PIN, !MODEM_RESET_LEVEL); delay(100);
// 	digitalWrite(MODEM_RESET_PIN, MODEM_RESET_LEVEL); delay(2600);
// 	digitalWrite(MODEM_RESET_PIN, !MODEM_RESET_LEVEL);
// #endif

// 	pinMode(BOARD_PWRKEY_PIN, OUTPUT);
// 	digitalWrite(BOARD_PWRKEY_PIN, LOW);
//   delay(100);
// 	digitalWrite(BOARD_PWRKEY_PIN, HIGH);
// 	delay(100);
// 	digitalWrite(BOARD_PWRKEY_PIN, LOW);

// 	// Check if the modem is online
// 	Serial.println("Start modem...");
// 	delay(3000);

// 	gps.begin();

// 	SerialAT.begin(UART_BAUD, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);

// 	// if (!modem.init())
// 	// {
// 	//     DBG("Failed to restart modem, delaying 10s and retrying");
// 	//     return;
// 	// }

// 	int retry = 0;
// 	while (!modem.testAT(1000)) {
// 			Serial.println(".");
// 			if (retry++ > 10) {
// 					digitalWrite(BOARD_PWRKEY_PIN, LOW);
// 					delay(100);
// 					digitalWrite(BOARD_PWRKEY_PIN, HIGH);
// 					delay(1000);
// 					digitalWrite(BOARD_PWRKEY_PIN, LOW);
// 					retry = 0;
// 			}
// 	}
// 	Serial.println();

// 			// Check if SIM card is online
// 	SimStatus sim = SIM_ERROR;
// 	while (sim != SIM_READY) {
// 		sim = modem.getSimStatus();
// 		switch (sim) {
// 		case SIM_READY:
// 			Serial.println("SIM card online");
// 			break;
// 		case SIM_LOCKED:
// 			Serial.println("The SIM card is locked. Please unlock the SIM card first.");
// 			// const char *SIMCARD_PIN_CODE = "123456";
// 			// modem.simUnlock(SIMCARD_PIN_CODE);
// 			break;
// 		default:
// 			break;
// 		}
// 		delay(1000);
// 	}


// 	if (!modem.setNetworkMode(38)) {
// 		Serial.println("Set network mode failed!");
// 	}
// 	String mode = modem.getNetworkModes();
// 	Serial.print("Current network mode : ");
// 	Serial.println(mode);

// #ifdef NETWORK_APN
// 	Serial.printf("Set network apn : %s\n", NETWORK_APN);
// 	modem.sendAT(GF("+CGDCONT=1,\"IP\",\""), NETWORK_APN, "\"");
// 	if (modem.waitResponse() != 1) {
// 		Serial.println("Set network apn error !");
// 	}
// #endif

// if (!modem.isGprsConnected()) {
// 	Serial.println("GPRS not connected!");
// 	// Try modem.gprsConnect(apn, gprsUser, gprsPass);
// } else {
// 	Serial.println("GPRS connected!");
// }
// 	Serial.println("Modem connected to GPRS");
// 	String name = modem.getModemName();
// 	DBG("Modem Name:", name);

// 	String modemInfo = modem.getModemInfo();
// 	DBG("Modem Info:", modemInfo);
    
// 	Serial.print("IP Address: ");
// 	Serial.println(modem.localIP());

// 	Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);


// 	ssl_client.setInsecure();
// 	ssl_client.setDebugLevel(1);
// 	ssl_client.setBufferSizes(4096 /* rx */, 2048 /* tx */);
// 	ssl_client.setClient(&gsm_client);

// 	stream_ssl_client.setInsecure();
// 	stream_ssl_client.setDebugLevel(1);
// 	stream_ssl_client.setBufferSizes(4096 /* rx */, 2048 /* tx */);
// 	stream_ssl_client.setClient(&stream_gsm_client);

// 	Serial.println("Initializing app...");
// 	initializeApp(aClient, app, getAuth(user_auth), auth_debug_print, "🔐 authTask");

// 	// Or intialize the app and wait.
// 	// initializeApp(aClient, app, getAuth(user_auth), 120 * 1000, auth_debug_print);

// 	app.getApp<RealtimeDatabase>(Database);

// 	Database.url(DATABASE_URL);

// 	// In SSE mode (HTTP Streaming) task, you can filter the Stream events by using AsyncClientClass::setSSEFilters(<keywords>),
// 	// which the <keywords> is the comma separated events.
// 	// The event keywords supported are:
// 	// get - To allow the http get response (first put event since stream connected).
// 	// put - To allow the put event.
// 	// patch - To allow the patch event.
// 	// keep-alive - To allow the keep-alive event.
// 	// cancel - To allow the cancel event.
// 	// auth_revoked - To allow the auth_revoked event.
// 	// To clear all prevousely set filter to allow all Stream events, use AsyncClientClass::setSSEFilters().
// 	streamClient.setSSEFilters("get,put,patch,keep-alive,cancel,auth_revoked");

// 	// The "unauthenticate" error can be occurred in this case because we don't wait
// 	// the app to be authenticated before connecting the stream.
// 	// This is ok as stream task will be reconnected automatically when the app is authenticated.

// 	Database.get(streamClient, "/examples/Stream/data", processData, true /* SSE mode */, "streamTask");

// 	// Async call with AsyncResult for returning result.
// 	// Database.get(streamClient, "/examples/Stream/data", streamResult, true /* SSE mode */);
// }

// void loop(){
//   // The async task handler should run inside the main loop
//   // without blocking delay or bypassing with millis code blocks.
//   gps.feed();

//   app.loop();

//   Database.loop();

//   if (millis() - ms > 45000 && app.ready()){

//     ms = millis();
    
//     JsonWriter writer;

//   if (gps.locationValid()) {
//   // Use fresh data if updated
//     if (gps.locationUpdated()){
//       if(gps.speed() > speedLimit){
//         overSpeed = true;
//       } else {
//         overSpeed = false;
//       }

//     GPSStringData gpsData = gps.getStringData();

//     Serial.println("Lat: " + gpsData.latitude);
//     Serial.println("Lng: " + gpsData.longitude);
//     Serial.println("Speed: " + gpsData.speedKmph + " km/h");
//     Serial.println("Time: " + gpsData.hour + ":" + gpsData.minute + ":" + gpsData.second);
//     Serial.println("Date: " + gpsData.year + "-" + gpsData.month + "-" + gpsData.day);

//     object_t json, obj1, obj2, obj3, obj4, obj5, obj6;
    
//     writer.create(obj1, "lat", gpsData.latitude);
//     writer.create(obj2, "lon", gpsData.longitude);
//     writer.create(obj3, "speed", gpsData.speedKmph);
//     writer.create(obj4, "overSpeed", overSpeed ? "true" : "false");
//     // writer.create(obj5, "time", gpsData.hour + ":" + gpsData.minute + ":" + gpsData.second);
//     // writer.create(obj6, "date", gpsData.year + "-" + gpsData.month + "-" + gpsData.day);

//     writer.join(json, 4, obj1, obj2, obj3, obj4);

//     Database.set<object_t>(aClient, "/examples/Stream/data", json, processData, "setTask");
//     }

//   }
//   //  else {
//   //   object_t json, obj1;
//   //   writer.create(obj1, "NoGps", "No fix available, sending default data");
//   //     // No valid fix yet: send placeholder or skip
//   //     Serial.println("No fix available, sending default data");

//   //     writer.join(json, 1, obj1);

//   //     Database.set<object_t>(aClient, "/examples/Stream/data", json, processData, "setTask");
//     // e.g., send zeros or last known coords
//   // }

//   }

//   if (millis() - gp > 800){

//     gp = millis();
    
//     if (gps.locationValid()) {
//       if (gps.locationUpdated()){

//         JsonWriter writer;
//         if(gps.speed() > speedLimit){

//           overSpeed = true;
//           Serial.println("Over speed detected! Speed: " + String(gps.speed()) + " km/h");

//            GPSStringData gpsData = gps.getStringData();

//             Serial.println("Lat: " + gpsData.latitude);
//             Serial.println("Lng: " + gpsData.longitude);
//             Serial.println("Speed: " + gpsData.speedKmph + " km/h");
//             Serial.println("Time: " + gpsData.hour + ":" + gpsData.minute + ":" + gpsData.second);
//             Serial.println("Date: " + gpsData.year + "-" + gpsData.month + "-" + gpsData.day);

//             object_t json, obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8;
            
//             writer.create(obj1, "lat", gpsData.latitude);
//             writer.create(obj2, "lon", gpsData.longitude);
//             writer.create(obj3, "speed", gpsData.speedKmph);
//             writer.create(obj4, "overSpeed", overSpeed ? "true" : "false");
//             // writer.create(obj5, "time", gpsData.hour + ":" + gpsData.minute + ":" + gpsData.second);
//             // writer.create(obj6, "date", gpsData.year + "-" + gpsData.month + "-" + gpsData.day);

//             writer.join(json, 4, obj1, obj2, obj3, obj4);

//             Database.set<object_t>(aClient, "/examples/Stream/data", json, processData, "setTask");
          
//         }else {
//           overSpeed = false;
//         }
//       }
//     }

//   }
//   delay(50);
// }


// void processData(AsyncResult &aResult){
// 	// Exits when no result available when calling from the loop.
// 	if (!aResult.isResult())
// 			return;

// 	if (aResult.isEvent()){
// 		Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.eventLog().message().c_str(), aResult.eventLog().code());
// 	}

// 	if (aResult.isDebug()){
// 		Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
// 	}

// 	if (aResult.isError()){
// 		Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
// 	}

// 	if (aResult.available()){
// 		RealtimeDatabaseResult &RTDB = aResult.to<RealtimeDatabaseResult>();
// 		if (RTDB.isStream()){
// 			Serial.println("----------------------------");
// 			Firebase.printf("task: %s\n", aResult.uid().c_str());
// 			Firebase.printf("event: %s\n", RTDB.event().c_str());
// 			Firebase.printf("path: %s\n", RTDB.dataPath().c_str());
// 			Firebase.printf("data: %s\n", RTDB.to<const char *>());
// 			Firebase.printf("type: %d\n", RTDB.type());

// 			// The stream event from RealtimeDatabaseResult can be converted to the values as following.
// 			bool v1 = RTDB.to<bool>();
// 			int v2 = RTDB.to<int>();
// 			float v3 = RTDB.to<float>();
// 			double v4 = RTDB.to<double>();
// 			String v5 = RTDB.to<String>();
// 		}
// 		else{
// 			Serial.println("----------------------------");
// 			Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
// 		}

// 		Firebase.printf("Free Heap: %d\n", ESP.getFreeHeap());
// 	}
// }