/**
 * ABOUT:
 *
 * The example to delete a Firestore composite index.
 *
 * This example uses the ServiceAuth class for authentication.
 * See examples/App/AppInitialization for more authentication examples.
 *
 * The OAuth2.0 authentication or access token authorization is required for indexing operations.
 *
 * The complete usage guidelines, please read README.md or visit https://github.com/mobizt/FirebaseClient
 *
 * SYNTAX:
 *
 * 1.------------------------
 *
 * Firestore::CollectionGroups::Indexes::deleteIndex(<AsyncClient>, <Firestore::Parent>, <collectionId>, <indexId>, <AsyncResultCallback>, <uid>);
 *
 * <AsyncClient> - The async client.
 * <Firestore::Parent> - The Firestore::Parent object included project Id and database Id in its constructor.
 * <collectionId> - The collection Id.
 * <indexId> - The index.
 * <AsyncResultCallback> - The async result callback (AsyncResultCallback).
 * <uid> - The user specified UID of async result (optional).
 *
 * The Firebase project Id should be only the name without the firebaseio.com.
 * The Firestore database id should be (default) or empty "".
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
void delete_async(const String &collectionId, const String &indexId);
void delete_async2(const String &collectionId, const String &indexId);
void delete_await(const String &collectionId, const String &indexId);

// ServiceAuth is required for Google Cloud Functions functions.
ServiceAuth sa_auth(FIREBASE_CLIENT_EMAIL, FIREBASE_PROJECT_ID, PRIVATE_KEY, 3000 /* expire period in seconds (<= 3600) */);

FirebaseApp app;

SSL_CLIENT ssl_client;

// This uses built-in core WiFi/Ethernet for network connection.
// See examples/App/NetworkInterfaces for more network examples.
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);

Firestore::CollectionGroups::Indexes indexes;

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

    app.getApp<Firestore::CollectionGroups::Indexes>(indexes);
}

void loop()
{
    // To maintain the authentication and async tasks
    app.loop();

    if (app.ready() && !taskCompleted)
    {
        taskCompleted = true;

        // This index Id is obtained from the index of the result of index creation payload.
        // e.g. projects/xxxxxxxx/databases/(default)/collectionGroups/yyyyy/indexes/zzzzz
        String collectionId = "a0";
        String indexId = "zzzzz";

        delete_async(collectionId, indexId);

        // delete_async2(collectionId, indexId);

        // delete_await(collectionId, indexId);
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

void delete_async(const String &collectionId, const String &indexId)
{
    Serial.println("Deleting a composite index... ");

    // Async call with callback function.
    indexes.deleteIndex(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), collectionId, indexId, processData, "deleteIndexTask");
}

void delete_async2(const String &collectionId, const String &indexId)
{
    Serial.println("Deleting a composite index... ");

    // Async call with AsyncResult for returning result.
    indexes.deleteIndex(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), collectionId, indexId, firestoreResult);
}

void delete_await(const String &collectionId, const String &indexId)
{
    Serial.println("Deleting a composite index... ");

    // Sync call which waits until the payload was received.
    String payload = indexes.deleteIndex(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), collectionId, indexId);
    if (aClient.lastError().code() == 0)
        Serial.println(payload);
    else
        Firebase.printf("Error, msg: %s, code: %d\n", aClient.lastError().message().c_str(), aClient.lastError().code());
}