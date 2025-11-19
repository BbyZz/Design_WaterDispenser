#include  <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>


LiquidCrystal_I2C lcd(0x27,20,4); 
SoftwareSerial espSerial(10, 11); // RX (to ESP TX), TX (to ESP RX)



// Intervals between Pulse
int  i=0;
// Pulse provided by Coin Acceptor
int impulsCount=0;
// Total Balance
int total_amount=0;

int clock = 8;

// Button pin and read
int btnML = 4; 
int btnL = 5; 
int btnG = 6;
int CANCEL = 7;

int ReadbtnML  = 0;
int ReadbtnL   = 0;
int ReadbtnG   = 0;
int ReadCANCEL = 0; 

// relays
int relaySolenoid =  12;
int relayCoin = 13;

// Water Flow sensor pins
const int flowSensorPin = 3;
const int coinPin = 2;

float calibrationFactor = 4.5;
volatile int pulseCount;  
float flowRate;
unsigned long flowMilliLitres;
unsigned long totalMilliLitres;
unsigned long oldTime;

void setup() {
 
  Serial.begin(9600);
  espSerial.begin(9600);

  lcd.init(); 
  lcd.backlight();
  pinMode(btnML, INPUT);
  pinMode(btnL, INPUT);
  pinMode(btnG, INPUT);
  pinMode(CANCEL, INPUT);

  pinMode(clock, OUTPUT);
  digitalWrite(clock, HIGH);


  lcd.setCursor(1,0);
  lcd.print("Water Dispense");
  lcd.setCursor(1,1);
  lcd.print("By: CJ CK BANS");
  
  pinMode(relaySolenoid,  OUTPUT);
  digitalWrite(relaySolenoid, HIGH);

  pinMode(relayCoin,  OUTPUT);
  digitalWrite(relayCoin, HIGH);

  pinMode(flowSensorPin, INPUT_PULLUP);

  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;

 attachInterrupt(digitalPinToInterrupt(flowSensorPin), pulseCounter, FALLING);

 // Interrupt connected  to PIN D2 executing IncomingImpuls function when signal goes from HIGH to LOW
  attachInterrupt(digitalPinToInterrupt(coinPin),incomingImpuls, FALLING); // 0= digital pin 2 1 = digital pin 3
  EEPROM.get(0, total_amount);
  
  

}

void incomingImpuls()
{
  impulsCount=impulsCount+1;
  i=0;
}

void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}

void lcdUpdateBal(int total_amount){
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("Current Balance: ");
  lcd.setCursor(1,1);
  lcd.print(total_amount);

}

void lcdDoneDispensing(){
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("Done");
  lcd.setCursor(1,1);
  lcd.print("Dispensing!");
  delay(2000);
}

void loop() {
  i=i+1;
  ReadbtnML = digitalRead(btnML);  
  ReadbtnL = digitalRead(btnL);
  ReadbtnG = digitalRead(btnG);
  ReadCANCEL = digitalRead(CANCEL);


  Serial.print("i=");
  Serial.print(i);
  Serial.print(" Impulses:");
  Serial.print(impulsCount);
  Serial.print(" Total:");
  Serial.println(total_amount);

  if (ReadbtnML == HIGH) { //minus 100 pesos
    delay(1000);
    Serial.println("btnML is pressed.");
    countdown();
    handleDispense(500, 100);
    total_amount = total_amount - 100;
    dispenseWater(500);
  } 
  
  if (ReadbtnL == HIGH) { //minus 500 pesos
    delay(1000);
    Serial.println("btnL is pressed.");
    countdown();
    handleDispense(1000, 500);
    total_amount = total_amount- 500;
    dispenseWater(1000);
    
  }  
  
  if (ReadbtnG == HIGH) {// minus 1000 pesos
    delay(1000);
    Serial.println("btnG is pressed.");
    countdown();
    handleDispense(18927, 1000);
    total_amount = total_amount- 1000;
    dispenseWater(18927);
  }  
  
  if (ReadCANCEL == HIGH) {
    Serial.println("Cancel is pressed.");
    delay(1000);
    digitalWrite(relaySolenoid, LOW);
  }
 
   if (i>=30){
    total_amount=total_amount+impulsCount;
    impulsCount=0;
    i=0;
    EEPROM.put(0, total_amount);
    lcdUpdateBal(total_amount);
  }


}

void countdown() {
  pinMode(clock, OUTPUT);

  digitalWrite(clock, LOW); // LOW turns it to 5 then followed by the for loop which iterates till 0 and end it with HIGH, so the next countdown LOW agad para mag 5
  for (int i = 5; i >= 1; i--) {
    digitalWrite(clock, HIGH);
    delay(1000);
    digitalWrite(clock, LOW);  
  }
  digitalWrite(clock, HIGH);

  Serial.println("Countdown complete.");
}

void handleDispense(int volume, int cost) {
  Serial.print("Requested: ");
  Serial.print(volume);
  Serial.print("ml | Cost: ");
  Serial.print(cost);
  Serial.print(" | Balance: ");
  Serial.println(total_amount);

  if (total_amount >= cost) {
    total_amount -= cost;

    // Send to ESP8266
    String msg = "DISPENSED," + String(volume) + "," + String(cost) + "," + String(total_amount);
    espSerial.println(msg);
    Serial.println("Sent to ESP: " + msg);
  } else {
    // Not enough balance
    String msg = "FAILED," + String(volume) + "," + String(cost) + "," + String(total_amount);
    espSerial.println(msg);
    Serial.println("Sent to ESP: " + msg);
  }
}


void dispenseWater(float totalVolume) {
  int targetMilliLitres = int(totalVolume); // Convert target volume to mL
  totalMilliLitres = 0;
  pulseCount = 0;
  oldTime = millis(); // Reset timing
  digitalWrite(relaySolenoid, HIGH); // Start dispensing

  Serial.print("Dispensing: ");
  Serial.println(totalVolume);

  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Dispensing:");
  lcd.setCursor(1, 1);
  lcd.print(totalVolume, 0); // Rounded display of volume

  while (totalMilliLitres < targetMilliLitres) {
    //
    if (digitalRead(CANCEL) == HIGH) {
      Serial.println("Cancel button pressed. Stopping...");
      digitalWrite(relaySolenoid, LOW);
      delay(100); // Small debounce
      break;
    }

    // Every second, update flow rate and total volume
    if ((millis() - oldTime) > 1000) {
      detachInterrupt(digitalPinToInterrupt(flowSensorPin));

      // Calculate flow rate and volume
      flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
      oldTime = millis();


      float flowMilliLitresPerSecond = (flowRate * 1000) / 60;
      totalMilliLitres += flowMilliLitresPerSecond;

      // Print debug info
      Serial.print("Flow rate: ");
      Serial.print(int(flowMilliLitresPerSecond));
      Serial.print(" mL/sec\t");

      Serial.print("Output Liquid Quantity: ");
      Serial.print(totalMilliLitres);
      Serial.println(" mL");

      // Reset pulse counter and re-enable interrupt
      pulseCount = 0;
      attachInterrupt(digitalPinToInterrupt(flowSensorPin), pulseCounter, FALLING);
    }
  }

  digitalWrite(relaySolenoid, LOW); // Stop dispensing
  Serial.println("Dispense complete.");
}