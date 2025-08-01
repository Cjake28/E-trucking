/**
 * ABOUT:
 *
 * The example for listing the ruleset metadata.
 *
 * This example uses the ServiceAuth class for authentication.
 * See examples/App/AppInitialization for more authentication examples.
 *
 * The OAuth2.0 authentication or access token authorization is required for security rules operations.
 *
 * The complete usage guidelines, please read README.md or visit https://github.com/mobizt/FirebaseClient
 *
 * SYNTAX:
 *
 * 1.------------------------
 *
 * RuleSets::list(<AsyncClient>, <projectId>, <ListOptions>, <AsyncResultCallback>, <uid>);
 *
 * <AsyncClient> - The async client.
 * <projectId> - The project Id.
 * <ListOptions> - The ListOptions which included ::filter, ::pageSize and ::pageToken.
 * - filter is the Ruleset filter. The list method supports filters with restrictions on Ruleset.name.
 * Filters on Ruleset.create_time should use the date function which parses strings that conform to the RFC 3339 date/time specifications.
 * Example: createTime > date("2017-01-01T00:00:00Z") AND name=UUID-*
 * - pageSize is the page size to load. Maximum of 100. Defaults to 10.
 * Note: pageSize is just a hint and the service may choose to load less than pageSize due to the size of the output.
 * To traverse all of the releases, caller should iterate until the pageToken is empty.
 * - pageToken is the next page token for loading the next batch of Ruleset instances.
 * <AsyncResultCallback> - The async result callback (AsyncResultCallback).
 * <uid> - The user specified UID of async result (optional).
 *
 * The Firebase project Id should be only the name without the firebaseio.com.
 *
 */

// To define build options in your sketch,
// adding the following macros before FirebaseClient.h
#define ENABLE_USER_CONFIG
#define ENABLE_SERVICE_AUTH
#define ENABLE_RULESETS

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
void list_ruleset_async(ListOptions &listOptions);
void list_ruleset_async2(ListOptions &listOptions);
void list_ruleset_await(ListOptions &listOptions);

// ServiceAuth is required for Google Cloud Functions functions.
ServiceAuth sa_auth(FIREBASE_CLIENT_EMAIL, FIREBASE_PROJECT_ID, PRIVATE_KEY, 3000 /* expire period in seconds (<= 3600) */);

FirebaseApp app;

SSL_CLIENT ssl_client;

// This uses built-in core WiFi/Ethernet for network connection.
// See examples/App/NetworkInterfaces for more network examples.
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);

RuleSets ruleSets;

AsyncResult rulesResult;

bool taskComplete = false;

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

    app.getApp<RuleSets>(ruleSets);
}

void loop()
{
    // To maintain the authentication and async tasks
    app.loop();

    if (app.ready() && !taskComplete)
    {
        taskComplete = true;

        ListOptions listOptions;

        // listOptions.filter("create_time > date(\"2017-01-01T00:00:00Z\") AND name=*");
        // listOptions.pageSize(5);
        // listOptions.pageToken("...");

        list_ruleset_async(listOptions);

        // list_ruleset_async2(listOptions);

        // list_ruleset_await(listOptions);
    }

    // For async call with AsyncResult.
    processData(rulesResult);
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

void list_ruleset_async(ListOptions &listOptions)
{
    Serial.println("Listing the ruleset metadata...");

    // Async call with callback function.
    ruleSets.list(aClient, FIREBASE_PROJECT_ID, listOptions, processData, "ruleSetsTask");
}

void list_ruleset_async2(ListOptions &listOptions)
{
    Serial.println("Listing the ruleset metadata...");

    // Async call with AsyncResult for returning result.
    ruleSets.list(aClient, FIREBASE_PROJECT_ID, listOptions, rulesResult);
}

void list_ruleset_await(ListOptions &listOptions)
{
    Serial.println("Listing the ruleset metadata...");

    // Sync call which waits until the payload was received.
    String payload = ruleSets.list(aClient, FIREBASE_PROJECT_ID, listOptions);
    if (aClient.lastError().code() == 0)
        Serial.println(payload);
    else
        Firebase.printf("Error, msg: %s, code: %d\n", aClient.lastError().message().c_str(), aClient.lastError().code());
}