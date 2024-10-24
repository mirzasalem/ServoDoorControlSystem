#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>

Servo myServo;          // Create a servo object
const int buttonPin = 2; // Push button connected to GPIO2 (adjust based on your wiring)
int buttonState = 0;     // Variable to store button state
int lastButtonState = 0; // To store previous button state
int servoAngle = 0;      // Servo angle starts at 0
bool increasing = true;  // Flag to control whether increasing or resetting

const char* ssid = "CIS Tech Ltd.";
const char* password = "cis@2022#";

// URL for PUT requests
String putUrl = "http://192.168.68.129:9000/jobs/door_status/";

AsyncWebServer server(80);  // Create an instance of the AsyncWebServer on port 80

// Variable to track door state
int doorState = 0; // Initial door state is 0
bool isJobReceived = false; // To track when the job data is received
int jobId = 0; // Variable to store job ID from the JSON data

void setup() {
  Serial.begin(115200);  // Start the Serial Monitor for debugging
  myServo.attach(4);                      // Attach the servo to GPIO4 (adjust based on your wiring)
  pinMode(buttonPin, INPUT_PULLUP);        // Use internal pull-up resistor for the button
  myServo.write(servoAngle);               // Start servo at 0 degrees

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Define route to handle POST request
  server.on("/sendjobdata", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "POST received");  // Acknowledge POST request
  }, NULL, handlePostJSON);  // Handle the POST body in this callback

  // Start the server
  server.begin();
}

// Function to handle incoming POST JSON data
void handlePostJSON(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  StaticJsonDocument<512> jsonDoc;  // Create a JSON document to store incoming data
  DeserializationError error = deserializeJson(jsonDoc, data, len);  // Parse the received data

  if (error) {
    Serial.println("Failed to parse JSON");
    request->send(400, "application/json", "{\"error\": \"Invalid JSON\"}");
    return;
  }

  // Extract values from JSON
  int status = jsonDoc["status"];
  const char* msg = jsonDoc["msg"];
  JsonObject dataObj = jsonDoc["data"];
  jobId = dataObj["id"];  // Store the job ID
  int door_open = dataObj["door_open"];
  
  Serial.printf("Received door_open value: %d\n", door_open);
  Serial.printf("Received user value: %d\n", jobId);

  // Flag that job is received and start cycling the door state
  isJobReceived = true;

  // Respond with success message
  request->send(200, "application/json", "{\"message\": \"Data received successfully\"}");
}

// Function to send PUT request with updated door state
void sendDoorState(int state) {
  HTTPClient http;
  String fullPutUrl = putUrl + String(jobId);  // Construct URL with jobId

  // Print the constructed URL
  Serial.println("Constructed PUT URL: " + fullPutUrl);

  http.begin(fullPutUrl);

  // Prepare JSON data for the PUT request
  String jsonData = "{\"door_open\":" + String(state) + "}";

  http.addHeader("Content-Type", "application/json"); // Specify content type header
  int putResponseCode = http.PUT(jsonData);

  if (putResponseCode > 0) {
    String putResponse = http.getString();
    Serial.println("HTTP PUT Response code: " + String(putResponseCode));
    Serial.println("PUT Response: " + putResponse);
  } else {
    Serial.println("Error on sending PUT: " + String(putResponseCode));
  }

  http.end();
}

// Function to cycle door_open values: 1
void cycleDoorStateOpen() {
    int state = 1;
    sendDoorState(state);  // Send current state
    Serial.printf("Sending door open: %d\n", state);
    delay(1000);           // Wait for 5 seconds before changing state
}


// Function to cycle door_open values: 2
void cycleDoorStateClose() {
    int state = 2;
    sendDoorState(state);  // Send current state
    Serial.printf("Sending door closed: %d\n", state);
    delay(1000);           // Wait for 5 seconds before changing state
    isJobReceived = false;  // Reset flag after cycling the states

  }


void loop() {
  // Once the job data is received, start cycling the door state
  if (isJobReceived) {
    buttonState = digitalRead(buttonPin);    // Read button state

    if (buttonState == LOW && lastButtonState == HIGH) {
      // Button has just been pressed
      handleServoMovement();
      // isJobReceived = false;  // Reset flag after cycling the states
      if (buttonState == HIGH && lastButtonState == LOW) {
      // Button has just been released
      Serial.println("Button released.");
            // You can add any additional actions when the button is released here
  }
    }
    

    lastButtonState = buttonState;  // Update last button state
    
  }
}

void handleServoMovement() {
  if (increasing) {
    increaseServoAngle();                   // Call function to increase servo angle
    cycleDoorStateOpen();
  } else {
    resetServo();
  }
}

void increaseServoAngle() {
  for (int i = 80; i <= 150; i += 2) {        // Increase servo angle by 5 degrees every second until 90°
    myServo.write(i);
    delay(100);                             // Delay for smooth movement
  }
  increasing = false;                       // Set to reset mode after reaching 90°
}

void resetServo() {
  for (int i = 140; i >= 80; i -= 2) {        // Increase servo angle by 5 degrees every second until 90°
    myServo.write(i);
    delay(100);                             // Delay for smooth movement
  }                         // Reset servo to 0 degrees
  increasing = true;
  cycleDoorStateClose();                           // Call function to reset servo angle
                          // Set to increase mode for the next press
}
