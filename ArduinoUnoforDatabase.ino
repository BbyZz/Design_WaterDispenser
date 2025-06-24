#include <SoftwareSerial.h>
SoftwareSerial espSerial(10, 11); // RX (to ESP TX), TX (to ESP RX)

int bal = 100;

const int button50 = 5;
const int button500 = 6;
const int button5000 = 7;

void setup() {
  pinMode(button50, INPUT);
  pinMode(button500, INPUT);
  pinMode(button5000, INPUT);

  espSerial.begin(9600);
  Serial.begin(9600); // For debugging
  Serial.println("Dispenser Ready");
}

void loop() {
  if (digitalRead(button50) == HIGH) {
    handleDispense(50, 5);
    delay(1000); // debounce
  }

  if (digitalRead(button500) == HIGH) {
    handleDispense(500, 10);
    delay(1000);
  }

  if (digitalRead(button5000) == HIGH) {
    handleDispense(5000, 15);
    delay(1000);
  }
}

void handleDispense(int volume, int cost) {
  Serial.print("Requested: ");
  Serial.print(volume);
  Serial.print("ml | Cost: ");
  Serial.print(cost);
  Serial.print(" | Balance: ");
  Serial.println(bal);

  if (bal >= cost) {
    bal -= cost;

    // Send to ESP8266
    String msg = "DISPENSED," + String(volume) + "," + String(cost) + "," + String(bal);
    espSerial.println(msg);
    Serial.println("Sent to ESP: " + msg);
  } else {
    // Not enough balance
    String msg = "FAILED," + String(volume) + "," + String(cost) + "," + String(bal);
    espSerial.println(msg);
    Serial.println("Sent to ESP: " + msg);
  }
}
