/**
 * ABOUT:
 *
 * The example to delete a Firestore database.
 *
 * This example uses the ServiceAuth class for authentication.
 * See examples/App/AppInitialization for more authentication examples.
 *
 * The OAuth2.0 authentication or access token authorization is required for Firestore database operations.
 *
 * The complete usage guidelines, please read README.md or visit https://github.com/mobizt/FirebaseClient
 *
 * SYNTAX:
 *
 * 1.------------------------
 *
 * Firestore::Databases::deleteDatabase(<AsyncClient>, <Firestore::Parent>, <etag>, <AsyncResultCallback>, <uid>);
 *
 * <AsyncClient> - The async client.
 * <Firestore::Parent> - The Firestore::Parent object included project Id and database Id in its constructor.
 * <etag> - The current etag of the Database.
 * <AsyncResultCallback> - The async result callback (AsyncResultCallback).
 * <uid> - The user specified UID of async result (optional).
 *
 * The Firebase project Id should be only the name without the firebaseio.com.
 * The Firestore database id is the id of database to delete.
 *
 * If an etag is provided and does not match the current etag of the database,
 * deletion will be blocked and a FAILED_PRECONDITION error will be returned.
 */

// To define build options in your sketch,
// adding the following macros before FirebaseClient.h
#define ENABLE_USER_CONFIG
#define ENABLE_SERVICE_AUTH
#define ENABLE_FIRESTORE

// For network independent usage (disable all network features).
// #define DISABLE_NERWORKS

#include <FirebaseClient.h>
#include "ExampleFunctions.h" // Provides the functions used in the examples.

#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

#define FIREBASE_PROJECT_ID "PROJECT_ID"
#define FIREBASE_CLIENT_EMAIL "CLIENT_EMAIL"
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----XXXXXXXXXXXX-----END PRIVATE KEY-----\n";

void processData(AsyncResult &aResult);
void delete_async(const String &databaseId, const String &etag);
void delete_async2(const String &databaseId, const String &etag);
void delete_await(const String &databaseId, const String &etag);

// ServiceAuth is required for Google Cloud Functions functions.
ServiceAuth sa_auth(FIREBASE_CLIENT_EMAIL, FIREBASE_PROJECT_ID, PRIVATE_KEY, 3000 /* expire period in seconds (<= 3600) */);

FirebaseApp app;

SSL_CLIENT ssl_client;

// This uses built-in core WiFi/Ethernet for network connection.
// See examples/App/NetworkInterfaces for more network examples.
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);

Firestore::Databases Databases;

AsyncResult firestoreResult;

bool taskCompleted = false;

void setup()
{
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);

    set_ssl_client_insecure_and_buffer(ssl_client);

    // Assign the valid time only required for authentication process with ServiceAuth and CustomAuth.
    app.setTime(get_ntp_time());

    Serial.println("Initializing app...");
    initializeApp(aClient, app, getAuth(sa_auth), auth_debug_print, "🔐 authTask");

    // Or intialize the app and wait.
    // initializeApp(aClient, app, getAuth(sa_auth), 120 * 1000, auth_debug_print);

    app.getApp<Firestore::Databases>(Databases);
}

void loop()
{
    // To maintain the authentication and async tasks
    app.loop();

    if (app.ready() && !taskCompleted)
    {
        taskCompleted = true;

        String databaseId = "myDb";
        String etag;

        delete_async(databaseId, etag);

        // delete_async2(databaseId, etag);

        // delete_await(databaseId, etag);
    }

    // For async call with AsyncResult.
    processData(firestoreResult);
}

void processData(AsyncResult &aResult)
{
    // Exits when no result available when calling from the loop.
    if (!aResult.isResult())
        return;

    if (aResult.isEvent())
    {
        Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.eventLog().message().c_str(), aResult.eventLog().code());
    }

    if (aResult.isDebug())
    {
        Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
    }

    if (aResult.isError())
    {
        Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
    }

    if (aResult.available())
    {
        Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
    }
}

void delete_async(const String &databaseId, const String &etag)
{
    Serial.println("Deleting a database... ");

    // Async call with callback function.
    Databases.deleteDatabase(aClient, Firestore::Parent(FIREBASE_PROJECT_ID, databaseId /* database Id */), etag, processData, "deleteTask");
}

void delete_async2(const String &databaseId, const String &etag)
{
    Serial.println("Deleting a database... ");

    // Async call with AsyncResult for returning result.
    Databases.deleteDatabase(aClient, Firestore::Parent(FIREBASE_PROJECT_ID, databaseId /* database Id */), etag, firestoreResult);
}

void delete_await(const String &databaseId, const String &etag)
{
    Serial.println("Deleting a database... ");

    // Sync call which waits until the payload was received.
    String payload = Databases.deleteDatabase(aClient, Firestore::Parent(FIREBASE_PROJECT_ID, databaseId /* database Id */), etag);
    if (aClient.lastError().code() == 0)
        Serial.println(payload);
    else
        Firebase.printf("Error, msg: %s, code: %d\n", aClient.lastError().message().c_str(), aClient.lastError().code());
}
