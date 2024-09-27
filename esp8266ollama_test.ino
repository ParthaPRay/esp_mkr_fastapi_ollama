// This code works on generic ESP8266 (generic) module on Arduino IDE 2.3.2 Tools --- > ESP8266 Boards ---> NodeMCU 1.0 (ESP-12E Module)
// Connect the WiFi with mobile hotspot for better testing
// The code continuously request one prompt to the Ollama API and receives the response from Ollama.
// Longer httptimeout till 20 seconds
// Better json parsing twice
// This code goes to deep sleep after all prompt sending and receiving
// Version 4
// Change the Baud Rate during experiments
// Partha Pratim Ray
// 10 September, 2024 

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Arduino_JSON.h>

// Replace with your network credentials
const char* ssid = "Galaxy M134728";  // Your WiFi SSID
const char* password = "7908402850";  // Your WiFi password

// Ollama API URL (Assuming FastAPI server is running at this IP and port)
const String ollama_api_url = "http://192.168.197.79:5000/generate";  // Replace with your FastAPI server's IP address

// Array of prompts to send
String prompts[] = {
  "What is the capital of Italy?",
  "Who discovered gravity?",
  "In which year did World War II end?",
  "Is the Earth round?",
  "Can penguins fly?",
  "Is water liquid?",
  "What is 9 plus 1?",
  "What is half of 16?",
  "What is 7 minus 4?",
  "Define a cell.",
  "Define a poem.",
  "Define a computer.",
  "The Sun rises in the...",
  "Necessity is the mother of…",
  "Practice makes...",
  "How to cook rice?",
  "How to write a story?",
  "How to draw a picture?"
};

// Total number of prompts
int totalPrompts = sizeof(prompts) / sizeof(prompts[0]);
int currentPromptIndex = 0;  // Index to track the current prompt

unsigned long sendTime = 0;  // Time when prompt is sent
unsigned long receiveTime = 0;  // Time when response is received

WiFiClient client;  // Create WiFiClient object

void setup() {
  Serial.begin(115200);   // Change the Baud Rate during Experiments

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
}

void loop() {
  // Send prompts sequentially until all are done
  if (WiFi.status() == WL_CONNECTED && currentPromptIndex < totalPrompts) {
    // Send the current prompt
    sendPromptToOllama(prompts[currentPromptIndex]);
    
    // Move to the next prompt
    currentPromptIndex++;
  } else if (currentPromptIndex >= totalPrompts) {
    // All prompts have been sent and received, stop the ESP8266
    Serial.println("All prompts have been sent. Stopping ESP8266.");
    ESP.deepSleep(0);  // Put ESP8266 into deep sleep
  }

  // Wait for 5 seconds before sending the next prompt
  delay(5000);
}

// Function to send a prompt to Ollama and measure the time
void sendPromptToOllama(String prompt) {
  HTTPClient http;
  Serial.println("[HTTP] Beginning connection...");
  
  if (http.begin(client, ollama_api_url)) {
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(60000);  // Increase timeout to 60 seconds to allow for slower response from Raspberry Pi

    // Prepare JSON payload
    String payload = "{\"prompt\":\"" + prompt + "\", \"format\": \"json\"}";
    sendTime = millis();
    
    int httpResponseCode = http.POST(payload);
    if (httpResponseCode > 0) {
      receiveTime = millis();
      String response = http.getString();
      Serial.println("Response Without Parsing: " + response); // Print whole response to serial monitor
      unsigned long sendDuration = receiveTime - sendTime;
      unsigned long receiveDuration = millis() - receiveTime;

      // Parse JSON response
      JSONVar jsonResponse = JSON.parse(response);
      if (JSON.typeof(jsonResponse) == "undefined") {
        Serial.println("Parsing input failed!");
        // Handle parsing failure gracefully
        return;
      }

      // Extract the response field
      String nestedJsonString = (const char*) jsonResponse["response"];
      if (nestedJsonString.length() == 0) {
        Serial.println("Empty response.");
        return;
      }

      // Handle response
      Serial.println("Response received: " + nestedJsonString);
      Serial.print("Time taken to send the prompt: ");
      Serial.print(sendDuration);
      Serial.println(" ms");

      Serial.print("Time taken to receive the response: ");
      Serial.print(receiveDuration);
      Serial.println(" ms");

      Serial.println("-----------------------------");
    } else {
      // Print detailed HTTP error
      Serial.print("HTTP error: ");
      Serial.println(httpResponseCode);
      Serial.print("WiFi Client Status: ");
      Serial.println(WiFi.status());
    }

    http.end();
  } else {
    Serial.println("Unable to connect to the server.");
  }
}
