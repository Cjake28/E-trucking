/**
 * ABOUT:
 *
 * The example to send the user verification email.
 *
 * This example uses the UserAuth class for authentication.
 * See examples/App/AppInitialization for more authentication examples.
 *
 * The complete usage guidelines, please read README.md or visit https://github.com/mobizt/FirebaseClient
 *
 * SYNTAX:
 *
 * 1.------------------------
 *
 * verify(<AsyncClient>, <FirebaseApp>, <user_auth_data>, <AsyncResultCallback>, <uid>);
 *
 * <AsyncClient> - The async client.
 * <FirebaseApp> - The FirebaseApp class object to handle authentication/authorization task.
 * <user_auth_data> - The user auth data (user_auth_data) which holds the user credentials from USerAccount class.
 * <AsyncResultCallback> - The async result callback (AsyncResultCallback).
 * <uid> - The user specified UID of async result (optional).
 */

// To define build options in your sketch,
// adding the following macros before FirebaseClient.h
#define ENABLE_USER_CONFIG
#define ENABLE_USER_AUTH

// For network independent usage (disable all network features).
// #define DISABLE_NERWORKS

#include <FirebaseClient.h>
#include "ExampleFunctions.h" // Provides the functions used in the examples.

#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

#define API_KEY "Web_API_KEY"

FirebaseApp app;

SSL_CLIENT ssl_client;

// This uses built-in core WiFi/Ethernet for network connection.
// See examples/App/NetworkInterfaces for more network examples.
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);

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

    UserAccount user(API_KEY);

    // The id token obtained from app.token() when initialize app with UserAuth
    const char *idtoken = "id token";

    set_ssl_client_insecure_and_buffer(ssl_client);

    Serial.println("Sending user verification email...");
    verify(aClient, app, getAuth(user.email("user@gmail.com").idToken(idtoken)), auth_debug_print, "verifyTask");

    // Or send user verification email and wait.
    verify(aClient, app, getAuth(user.email("user@gmail.com").idToken(idtoken)), 120 * 1000, auth_debug_print);
}

void loop()
{
    // To maintain the authentication and async tasks.
    app.loop();
}