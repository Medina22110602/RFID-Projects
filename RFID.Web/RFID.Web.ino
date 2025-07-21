#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include <MFRC522Debug.h>
#include <map>

// Wi-Fi credentials
const char* ssid = "Intelory WiFi";
const char* password = "LuTa6OF0@INT";

// Web server on port 80
ESP8266WebServer server(80);

// RFID setup
MFRC522DriverPinSimple ss_pin(15);
MFRC522DriverSPI driver{ss_pin};
MFRC522 mfrc522{driver};
MFRC522::MIFARE_Key key;

// Block settings
byte blockAddress = 2;
byte blockDataRead[18];
byte bufferblocksize = 18;

// Map to store card UIDs and their data
std::map<String, String> cardData = {
    {"ca:90:23:b0", "Medina"},
    {"f3:26:7c:a9", "Admin"}
    
};

String scannedData = "Waiting for card...";

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Initialize RFID reader
  mfrc522.PCD_Init();
  Serial.println(F("RFID Reader Initialized."));

  // Prepare key
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Configure web server routes
  server.on("/", handleRoot);
  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  server.handleClient(); // Handle web server requests

  // Check for a new card
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(500);
    return;
  }

  // Read card UID
  Serial.print("Card UID: ");
  String cardUID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.printf("%02X", mfrc522.uid.uidByte[i]);
    cardUID += String(mfrc522.uid.uidByte[i], HEX);
    if (i < mfrc522.uid.size - 1) cardUID += ":";
  }
  Serial.println();

  // Look up the card's data
  if (cardData.find(cardUID) != cardData.end()) {
    scannedData = "Card UID: " + cardUID + "<br>Data: " + cardData[cardUID];
  } else {
    scannedData = "Card UID: " + cardUID + "<br>Data: Unknown card.";
  }

  // Halt communication with the card
  mfrc522.PICC_HaltA();
  delay(2000);
}

// Handle root path of the web server
void handleRoot() {
  String html = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
      <title>RFID Reader</title>
      <style>
          body {
              font-family: 'Arial', sans-serif;
              margin: 0;
              padding: 0;
              background: linear-gradient(to right, #4caf50, #81c784);
              color: #333;
              text-align: center;
          }
          header {
              background: linear-gradient(to bottom, #388e3c, #4caf50);
              color: white;
              padding: 20px 0;
              box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
          }
          header h1 {
              margin: 0;
              font-size: 3em;
              text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.3);
          }
          .content {
              padding: 20px;
          }
          .data-box {
              margin: 20px auto;
              padding: 20px;
              width: 80%;
              max-width: 400px;
              background: #ffffff;
              box-shadow: 0 8px 16px rgba(0, 0, 0, 0.2);
              border-radius: 10px;
              text-align: center;
          }
          .data-box h2 {
              color: #4caf50;
              font-size: 1.5em;
              margin: 0 0 10px;
          }
          .data-box p {
              font-size: 1.2em;
              color: #555;
              margin: 0;
          }
      </style>
  </head>
  <body>
      <header>
          <h1>RFID Reader Data</h1>
      </header>
      <div class="content">
          <div class="data-box">
              <h2>Scanned Data</h2>
              <p>)rawliteral" + scannedData + R"rawliteral(</p>
          </div>
      </div>
  </body>
  </html>
  )rawliteral";

  server.send(200, "text/html", html);
}



