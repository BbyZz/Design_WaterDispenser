#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

const char* ssid = "OnePlus 8T";
const char* password = "pogchamp1";

const char* scriptHost = "script.google.com";
const int scriptPort = 443;
const char* scriptPath = "/macros/s/AKfycbwceevIX1y2HQHF1Cime1sCBzck074mHWLC2LPBh7OyQjeeeciX8_2J5BG_4eXn_eWK/exec";

// D5 = RX (to Arduino TX), D6 = TX (to Arduino RX)
SoftwareSerial arduinoSerial(D5, D6);

void setup() {
  Serial.begin(9600);
  arduinoSerial.begin(9600);

  Serial.println("\nConnecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (arduinoSerial.available()) {
    String rawData = arduinoSerial.readStringUntil('\n');
    rawData.trim();
    if (rawData.length() > 0) {
      Serial.print("Received from Arduino: ");
      Serial.println(rawData);
      sendToGoogleSheet(rawData);
    }
  }
}

void sendToGoogleSheet(const String &data) {
  WiFiClientSecure client;
  client.setInsecure();

  if (!client.connect(scriptHost, scriptPort)) {
    Serial.println("Connection to Google failed");
    return;
  }

  String postData = "data=" + urlEncode(data);
  int contentLength = postData.length();

  client.print("POST ");
  client.print(scriptPath);
  client.println(" HTTP/1.1");

  client.print("Host: ");
  client.println(scriptHost);

  client.println("Content-Type: application/x-www-form-urlencoded");
  client.print("Content-Length: ");
  client.println(contentLength);
  client.println("Connection: close");
  client.println();

  client.print(postData);

  Serial.println("POSTing to Google Sheets...");
  Serial.println("Payload: " + postData);

  while (client.connected()) {
    if (client.available()) {
      String line = client.readStringUntil('\n');
      Serial.println("Response: " + line);
    }
  }

  client.stop();
}

String urlEncode(const String &str) {
  String encoded = "";
  char c;
  char code0, code1;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (isalnum(c)) {
      encoded += c;
    } else {
      code0 = (c >> 4) & 0xF;
      code1 = c & 0xF;
      encoded += '%';
      encoded += "0123456789ABCDEF"[code0];
      encoded += "0123456789ABCDEF"[code1];
    }
  }
  return encoded;
}
