#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Wi-Fi credentials
const char* ssid = "Your_WiFi_SSID";
const char* password = "Your_WiFi_Password";

// Endpoint URL
const char* endpoint = "https://rfid-attendance-system-h1uv.onrender.com/attendance";

// RFID pins
#define RST_PIN D3  // Reset pin
#define SS_PIN D4   // Slave Select pin

MFRC522 rfid(SS_PIN, RST_PIN); // Create an instance of the RFID reader

WiFiClient client;

void setup() {
  Serial.begin(115200);
  SPI.begin();          // Initialize SPI
  rfid.PCD_Init();      // Initialize RFID reader
  Serial.println("RFID Reader initialized.");

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi");
}

void loop() {
  // Look for an RFID card
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    delay(100);
    return;
  }

  // Read RFID UID
  String rfid_id = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    rfid_id += String(rfid.uid.uidByte[i], HEX); // Convert UID bytes to hex string
    if (i < rfid.uid.size - 1) {
      rfid_id += "_"; // Separate bytes with "_"
    }
  }
  Serial.println("RFID ID: " + rfid_id);

  // Send RFID ID to the server
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(client, endpoint);
    http.addHeader("Content-Type", "application/json");

    String jsonPayload = "{\"rfid_id\": \"" + rfid_id + "\"}";
    Serial.println("Sending POST request with payload: " + jsonPayload);

    int httpResponseCode = http.POST(jsonPayload);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("HTTP Response code: " + String(httpResponseCode));
      Serial.println("Response: " + response);
    } else {
      Serial.println("Error in HTTP request: " + String(httpResponseCode));
    }
    http.end();
  } else {
    Serial.println("Wi-Fi not connected!");
  }

  delay(2000); // Delay before the next read
}
