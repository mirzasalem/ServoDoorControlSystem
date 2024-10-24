#include <WiFi.h>
#include <HTTPClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <WebServer.h>
const char* ssid = "CIS Tech Ltd.";
const char* password = "cis@2022#";

const char* serverName = "http://192.168.68.108:9000/jobs/door_status/2";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  // Wait for WiFi to connect
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  // Create an HTTPClient instance
  HTTPClient http;

  // Prepare the JSON payload
  String jsonPayload;
  for (int door_open = 0; door_open <= 2; door_open++) {
    jsonPayload = String("{\"door_open\":") + door_open + "}";
    
    // Specify the server URL
    http.begin(serverName);

    // Specify content type
    http.addHeader("Content-Type", "application/json");

    // Send the PUT request
    int httpResponseCode = http.PUT(jsonPayload);

    // Check the response code
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.printf("HTTP Response code: %d\n", httpResponseCode);
      Serial.println("Response: " + response);
    } else {
      Serial.printf("Error in sending PUT request: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    // End the HTTP connection
    http.end();

    // Wait for 5 seconds before sending the next request
    delay(5000);
  }
}
