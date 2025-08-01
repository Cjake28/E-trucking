/**
 * ABOUT:
 *
 * The beare minimum code example for using all Firebase services.
 *
 * The steps which are generally required are explained below.
 *
 * Step 1. Include the network, SSL client and Firebase libraries.
 * ===============================================================
 *
 * Step 2. Define the user functions that requred for the library usage.
 * =====================================================================
 *
 * Step 3. Define the authentication config (identifier) class.
 * ============================================================
 * In the Firebase/Google Cloud services REST APIs, the auth tokens are used for authentication/authorization.
 *
 * The auth token is a short-lived token that will be expired in 60 minutes and need to be refreshed or re-created when it expired.
 *
 * There can be some special use case that some services provided the non-authentication usages e.g. using database secret
 * in Realtime Database, setting the security rules in Realtime Database, Firestore and Firebase Storage to allow public read/write access.
 *
 * The UserAuth (user authentication with email/password) is the basic authentication for Realtime Database,
 * Firebase Storage and Firestore services except for some Firestore services that involved with the Google Cloud services.
 *
 * It stores the email, password and API keys for authentication process.
 *
 * In Google Cloud services e.g. Cloud Storage and Cloud Functions, the higest authentication level is required and
 * the ServiceAuth class (OAuth2.0 authen) and AccessToken class will be use for this case.
 *
 * While the CustomAuth provides the same authentication level as user authentication unless it allows the custom UID and claims.
 *
 * Step 4. Define the authentication handler class.
 * ================================================
 * The FirebaseApp actually works as authentication handler.
 * It also maintains the authentication or re-authentication when you place the FirebaseApp::loop() inside the main loop.
 *
 * Step 5. Define the SSL client.
 * ==============================
 * It handles server connection and data transfer works.
 *
 * In this beare minimum example we use only one SSL client for all processes.
 * In some use cases e.g. Realtime Database Stream connection, you may have to define the SSL client for it separately.
 *
 * Step 6. Define the Async Client.
 * ================================
 * This is the class that is used with the functions where the server data transfer is involved.
 * It stores all sync/async taks in its queue.
 *
 * It requires the SSL client and network config (identifier) data for its class constructor for its network re-connection
 * (e.g. WiFi and GSM), network connection status checking, server connection, and data transfer processes.
 *
 * This makes this library reliable and operates precisely under various server and network conditions.
 *
 * Step 7. Define the class that provides the Firebase/Google Cloud services.
 * ==========================================================================
 * The Firebase/Google Cloud services classes provide the member functions that works with AsyncClient.
 *
 * Step 8. Start the authenticate process.
 * ========================================
 * At this step, the authentication credential will be used to generate the auth tokens for authentication by
 * calling initializeApp.
 *
 * This allows us to use different authentications for each Firebase/Google Cloud services with different
 * FirebaseApps (authentication handler)s.
 *
 * When calling initializeApp with timeout, the authenication process will begin immediately and wait at this process
 * until it finished or timed out. It works in sync mode.
 *
 * If no timeout was assigned, it will work in async mode. The authentication task will be added to async client queue
 * to process later e.g. in the loop by calling FirebaseApp::loop.
 *
 * The workflow of authentication process.
 *
 * -----------------------------------------------------------------------------------------------------------------
 *  Setup   |    FirebaseApp [account credentials/tokens] ───> InitializeApp (w/wo timeout) ───> FirebaseApp::getApp
 * -----------------------------------------------------------------------------------------------------------------
 *  Loop    |    FirebaseApp::loop  ───> FirebaseApp::ready ───> Firebase Service API [auth token]
 * ---------------------------------------------------------------------------------------------------
 *
 * Step 9. Bind the FirebaseApp (authentication handler) with your Firebase/Google Cloud services classes.
 * ========================================================================================================
 * This allows us to use different authentications for each Firebase/Google Cloud services.
 *
 * It is easy to bind/unbind/change the authentication method for different Firebase/Google Cloud services APIs.
 *
 * Step 10. Set the Realtime Database URL (for Realtime Database only)
 * ===================================================================
 *
 * Step 11. Maintain the authentication and async tasks in the loop.
 * ==============================================================
 * This is required for authentication/re-authentication process and keeping the async task running.
 *
 * Step 12. Checking the authentication status before use.
 * =======================================================
 * Before calling the Firebase/Google Cloud services functions, the FirebaseApp::ready() of authentication handler that bined to it
 * should return true.
 *
 * Step 13. Process the results of async tasks the end of the loop.
 * ============================================================================
 * This requires only when async result was assigned to the Firebase/Google Cloud services functions.
 */

// Step 1

// To define build options in your sketch,
// adding the following macros before FirebaseClient.h
#define ENABLE_USER_CONFIG
#define ENABLE_SERVICE_AUTH
#define ENABLE_DATABASE
#define ENABLE_FIRESTORE
#define ENABLE_FIRESTORE_QUERY
#define ENABLE_MESSAGING
#define ENABLE_STORAGE
#define ENABLE_CLOUD_STORAGE
#define ENABLE_FUNCTIONS

// For network independent usage (disable all network features).
// #define DISABLE_NERWORKS

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>

// Step 2
void processData(AsyncResult &aResult);
void getMsg(Messages::Message &msg);

// Step 3
ServiceAuth sa_auth("CLIENT_EMAIL", "PROJECT_ID", "PRIVATE_KEY", 3000);

// Step 4
FirebaseApp app;

// Step 5
WiFiClientSecure ssl_client1, ssl_client2;

// Step 6
// Use two AsyncClients for sync and async tasks for demonstation only.
// This uses built-in core WiFi/Ethernet for network connection.
// See examples/App/NetworkInterfaces for more network examples.
using AsyncClient = AsyncClientClass;
AsyncClient async_client1(ssl_client1), async_client2(ssl_client2);

// Step 7
RealtimeDatabase Database;
Messaging messaging;
Firestore::Documents Docs;
Storage storage;
CloudStorage cstorage;
CloudFunctions cfunctions;

bool onetimeTest = false;

// The Optional proxy object that provides the data/information
//  when used in async mode without callback.
AsyncResult dbResult, fcmResult, firestoreResult, functionResult, storageResult, cloudStorageResult;

uint32_t getTime()
{
    uint32_t ts = 0;
    Serial.print("Getting time from NTP server... ");
#if defined(ESP8266) || defined(ESP32) || defined(CORE_ARDUINO_PICO)
    int max_try = 10, retry = 0;
    while (time(nullptr) < FIREBASE_DEFAULT_TS && retry < max_try)
    {
        configTime(3 * 3600, 0, "pool.ntp.org");
        unsigned long ms = millis();
        while (time(nullptr) < FIREBASE_DEFAULT_TS && millis() - ms < 10 * 1000)
        {
            delay(100);
        }
        Serial.print(ts == 0 ? " failed, retry... " : "");
        retry++;
    }
    ts = time(nullptr);
#elif __has_include(<WiFiNINA.h>) || __has_include(<WiFi101.h>)
    ts = WiFi.getTime();
#endif

    Serial.println(ts > 0 ? "success" : "failed");
    return ts;
}

void setup()
{
    Serial.begin(115200);

    WiFi.begin("WIFI_AP", "WIFI_PASSWORD");

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

    // The SSL client options depend on the SSL client used.

    // Skip certificate verification
    ssl_client1.setInsecure();
    ssl_client2.setInsecure();

    // Set timeout
    ssl_client1.setConnectionTimeout(1000);
    ssl_client1.setHandshakeTimeout(5);
    ssl_client2.setConnectionTimeout(1000);
    ssl_client2.setHandshakeTimeout(5);

    // ESP8266 Set buffer size
    // ssl_client1.setBufferSizes(4096, 1024);
    // ssl_client2.setBufferSizes(4096, 1024);

    // Assign the valid time only required for authentication process with ServiceAuth and CustomAuth.
    app.setTime(getTime());

    // Step 8
    initializeApp(async_client1, app, getAuth(sa_auth), processData, "🔐 authTask");

    // Step 9
    app.getApp<RealtimeDatabase>(Database);
    app.getApp<Messaging>(messaging);
    app.getApp<Firestore::Documents>(Docs);
    app.getApp<Storage>(storage);
    app.getApp<CloudStorage>(cstorage);
    app.getApp<CloudFunctions>(cfunctions);

    // Step 10
    Database.url("DATABASE_URL");
}

void loop()
{
    // Step 11
    app.loop();

    // Step 12
    if (app.ready() && !onetimeTest)
    {
        onetimeTest = true;

        // Realtime Database set value.
        // ============================

        // Async call with callback function
        Database.set<object_t>(async_client1, "/examples/BareMinimum/data/set4", object_t("{\"title\":\"Greeting!\",\"message\":\"Hi, I'm JSON.\"}"), processData, "RealtimeDatabase_SetTask");

        // Async call with AsyncResult for returning result.
        Database.set<number_t>(async_client1, "/examples/BareMinimum/data/set5", number_t(21.824976, 6), dbResult);

        // Sync call which waits until the result was received.
        bool status = Database.set<object_t>(async_client2, "/examples/BareMinimum/data/set6", object_t("[100,200,300]"));
        if (status)
            Serial.println("Value set complete.");
        else
            Firebase.printf("Error, msg: %s, code: %d\n", async_client2.lastError().message().c_str(), async_client2.lastError().code());

        Messages::Message msg;
        getMsg(msg);

        // Cloud Messaging send message.
        // =============================

        // Async call with callback function
        messaging.send(async_client1, Messages::Parent("PROJECT_ID"), msg, processData, "Messaging_SendMessageTask");

        // Async call with AsyncResult for returning result.
        messaging.send(async_client1, Messages::Parent("PROJECT_ID"), msg, fcmResult);

        // Sync call which waits until the result was received.
        String payload = messaging.send(async_client2, Messages::Parent("PROJECT_ID"), msg);

        if (async_client2.lastError().code() == 0)
            Serial.println(payload);
        else
            Firebase.printf("Error, msg: %s, code: %d\n", async_client2.lastError().message().c_str(), async_client2.lastError().code());

        // Firestore get document.
        // =======================

        // Async call with callback function
        Docs.get(async_client1, Firestore::Parent("PROJECT_ID"), "/examples/BareMinimum", GetDocumentOptions(DocumentMask("Singapore")), processData, "Firestore_GetDocumentTask");

        // Async call with AsyncResult for returning result.
        Docs.get(async_client1, Firestore::Parent("PROJECT_ID"), "/examples/BareMinimum", GetDocumentOptions(DocumentMask("Singapore")), firestoreResult);

        // Sync call which waits until the result was received.
        payload = Docs.get(async_client2, Firestore::Parent("PROJECT_ID"), "/examples/BareMinimum", GetDocumentOptions(DocumentMask("Singapore")));

        if (async_client2.lastError().code() == 0)
            Serial.println(payload);
        else
            Firebase.printf("Error, msg: %s, code: %d\n", async_client2.lastError().message().c_str(), async_client2.lastError().code());

        // Firebase Storage get object metadata
        // ====================================

        // Async call with callback function
        storage.getMetadata(async_client1, FirebaseStorage::Parent("STORAGE_BUCKET_ID", "/examples/BareMinimum/media.mp4"), processData, "Storage_GetMetadataTask");

        // Async call with AsyncResult for returning result.
        storage.getMetadata(async_client1, FirebaseStorage::Parent("STORAGE_BUCKET_ID", "/examples/BareMinimum/media.mp4"), storageResult);

        // Sync call which waits until the result was received.
        payload = storage.getMetadata(async_client2, FirebaseStorage::Parent("STORAGE_BUCKET_ID", "/examples/BareMinimum/media.mp4"));

        if (async_client2.lastError().code() == 0)
            Serial.println(payload);
        else
            Firebase.printf("Error, msg: %s, code: %d\n", async_client2.lastError().message().c_str(), async_client2.lastError().code());

        // Cloud Storage list objects.
        // ===========================

        GoogleCloudStorage::ListOptions options;
        options.maxResults(3);
        options.prefix("media");

        // Async call with callback function
        cstorage.list(async_client1, GoogleCloudStorage::Parent("STORAGE_BUCKET_ID"), options, processData, "CloudStorage_ListObjectTask");

        // Async call with AsyncResult for returning result.
        cstorage.list(async_client1, GoogleCloudStorage::Parent("STORAGE_BUCKET_ID"), options, cloudStorageResult);

        // Sync call which waits until the result was received.
        payload = cstorage.list(async_client2, GoogleCloudStorage::Parent("STORAGE_BUCKET_ID"), options);

        if (async_client2.lastError().code() == 0)
            Serial.println(payload);
        else
            Firebase.printf("Error, msg: %s, code: %d\n", async_client2.lastError().message().c_str(), async_client2.lastError().code());

        // Cloud Functions call function.
        // ==============================

        // Async call with callback function
        cfunctions.call(async_client1, GoogleCloudFunctions::Parent("PROJECT_ID", "PROJECT_LOCATION"), "helloWorld", "test", processData, "Functions_CallTask");

        // Async call with AsyncResult for returning result.
        cfunctions.call(async_client1, GoogleCloudFunctions::Parent("PROJECT_ID", "PROJECT_LOCATION"), "helloWorld", "test", functionResult);

        // Sync call which waits until the result was received.
        payload = cfunctions.call(async_client2, GoogleCloudFunctions::Parent("PROJECT_ID", "PROJECT_LOCATION"), "helloWorld", "test");
        if (async_client2.lastError().code() == 0)
            Serial.println(payload);
        else
            Firebase.printf("Error, msg: %s, code: %d\n", async_client2.lastError().message().c_str(), async_client2.lastError().code());
    }

    // Step 13
    processData(dbResult);
    processData(fcmResult);
    processData(firestoreResult);
    processData(functionResult);
    processData(storageResult);
    processData(cloudStorageResult);
}

void processData(AsyncResult &aResult)
{
    // Exits when no result available when calling from the loop.
    if (!aResult.isResult())
        return;

    if (aResult.isEvent())
        Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.eventLog().message().c_str(), aResult.eventLog().code());

    if (aResult.isDebug())
        Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());

    if (aResult.isError())
        Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());

    if (aResult.available())
        Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
}

void getMsg(Messages::Message &msg)
{
    msg.topic("test");

    Messages::Notification notification;
    notification.body("Notification body").title("Notification title");

    msg.notification(notification);
}
