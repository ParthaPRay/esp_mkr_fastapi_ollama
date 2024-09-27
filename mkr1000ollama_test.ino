// This code works on generic Arduino MKR1000 module on Arduino IDE 2.3.2 Tools --- > Arduino SAMD Boards ---> Arduino MKR1000
// Connect the WiFi with mobile hotspot for better testing
// The code continuously request one prompt to the Ollama API and receives the response from Ollama
// Longer httptimeout till 20 seconds
// Better json parsing twice
// Version 2
// This code goes to deep sleep when all prompts done with sending and receiving
// Partha Pratim Ray
// 11 September, 2024 
#include <SPI.h>
#include <WiFi101.h>
#include <Arduino_JSON.h>

// Replace with your network credentials
const char* ssid = "Galaxy M134728";  // Your WiFi SSID
const char* password = "7908402850";  // Your WiFi password

// Ollama API URL (Assuming FastAPI server is running at this IP and port)
const char* server = "192.168.197.79";  // Replace with your FastAPI server's IP address
const int port = 5000;  // Replace with your FastAPI server's port

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
bool allPromptsSent = false;  // Flag to track if all prompts have been sent

unsigned long sendTime = 0;  
unsigned long receiveTime = 0;  

WiFiClient client;  // Initialize WiFiClient

void setup() {
  Serial.begin(115200);
  
  // Check for the presence of the Wi-Fi shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);  // Stop here if no shield
  }

  // Connect to the Wi-Fi network
  connectToWiFi();

  Serial.println("\nStarting connection to server...");
}

void loop() {
  // Only process prompts if not all have been sent
  if (!allPromptsSent && WiFi.status() == WL_CONNECTED && currentPromptIndex < totalPrompts) {
    // Send the current prompt
    sendPromptToOllama(prompts[currentPromptIndex]);
    
    // Move to the next prompt
    currentPromptIndex++;
  } else if (currentPromptIndex >= totalPrompts && !allPromptsSent) {
    // All prompts have been sent and received, stop the ESP32
    Serial.println("All prompts have been sent. Entering deep sleep...");
    
    // Set the flag to true to prevent repetitive deep sleep calls
    allPromptsSent = true;

    // Go into deep sleep
    enterDeepSleep();
  }

  // Wait for 5 seconds before sending the next prompt
  delay(5000);
}

// Function to connect to Wi-Fi
void connectToWiFi() {
  int status = WL_IDLE_STATUS;  // WiFi connection status
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, password);

    // Wait 10 seconds for connection
    delay(10000);
  }
  Serial.println("Connected to WiFi");
  printWiFiStatus();
}

// Function to print WiFi details
void printWiFiStatus() {
  // Print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // Print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // Print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
}

// Function to send a prompt to Ollama API and measure the time
void sendPromptToOllama(String prompt) {
  Serial.println("[HTTP] Beginning connection...");

  // Connect to server on the specified port
  if (client.connect(server, port)) {
    Serial.println("Connected to server");

    // Prepare HTTP POST request
    String payload = "{\"prompt\":\"" + prompt + "\", \"format\": \"json\"}";
    String request = String("POST /generate HTTP/1.1\r\n") +
                     "Host: " + String(server) + "\r\n" +
                     "Content-Type: application/json\r\n" +
                     "Content-Length: " + String(payload.length()) + "\r\n" +
                     "Connection: close\r\n\r\n" + payload;

    sendTime = millis();  // Log the time when the request is sent

    // Send HTTP request
    client.println(request);
    Serial.println("HTTP Request sent:");
    Serial.println(request);

    // Wait for response from the server
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 30000) {  // Timeout if no response in 30 seconds
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }

    // Read the response from the server
    String response = "";
    bool headersEnded = false;
    while (client.available()) {
      String line = client.readStringUntil('\r');
      line.trim(); // Remove any extraneous white spaces

      // Check if we've reached the end of the headers
      if (line.length() == 0 && !headersEnded) {
        headersEnded = true; // Start reading the actual content
      } else if (headersEnded) {
        response += line;  // Only append lines after headers are done
      }
    }

    receiveTime = millis();  // Log the time when the response is received

    // Print the raw response
    Serial.print("Raw Response: ");
    Serial.println(response); 

    // Time measurements irrespective of parsing
    unsigned long sendDuration = receiveTime - sendTime;
    unsigned long receiveDuration = millis() - receiveTime;

    Serial.print("Time taken to send the prompt: ");
    Serial.print(sendDuration);
    Serial.println(" ms");

    Serial.print("Time taken to receive the response: ");
    Serial.print(receiveDuration);
    Serial.println(" ms");

    Serial.println("-----------------------------");

    // Parse the JSON part of the response
    JSONVar jsonResponse = JSON.parse(response);
    if (JSON.typeof(jsonResponse) == "undefined") {
      Serial.println("Parsing input failed!");
    } else {
      // Extract the response field from the JSON object
      String responseText = (const char*) jsonResponse["response"];
      if (responseText.length() == 0) {
        Serial.println("Empty response.");
      } else {
        // Handle the received response
        Serial.println("Response received: " + responseText);
      }
    }

    // Stop the client after receiving the response
    client.stop();
  } else {
    Serial.println("Connection failed!");
  }
}

// Function to enter deep sleep
void enterDeepSleep() {
  Serial.println("Entering deep sleep mode.");
  
  // Wait for some time to ensure message is printed before sleep
  delay(1000);

  // Use the internal sleep function for MKR1000
  // You can specify the time for sleep in seconds (optional)
  WiFi.end(); // Disconnect from WiFi
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;  // Set the SLEEPDEEP bit
  __WFI();  // Wait for interrupt to wake up the device
}
