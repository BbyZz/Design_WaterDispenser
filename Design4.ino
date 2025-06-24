#include  <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x27,20,4); 


// Intervals between Pulse
int  i=0;
// Pulse provided by Coin Acceptor
int impulsCount=0;
// Total Balance
int total_amount=0;

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

// Water Flow sensor pins
byte sensorInterrupt = 1;  // 0 = digital pin 2
byte sensorPin       = 3;


float calibrationFactor = 4.5;

volatile byte pulseCount;  

float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

unsigned long oldTime;

void setup() {
 
  Serial.begin(9600);
  lcd.init(); 
  lcd.backlight();
  pinMode(btnML, INPUT);
  pinMode(btnL, INPUT);
  pinMode(btnG, INPUT);
  pinMode(CANCEL, INPUT);

  lcd.setCursor(1,0);
  lcd.print("Water Dispense");
  lcd.setCursor(1,1);
  lcd.print("By: CJ CK BANS");

  pinMode(btnML, INPUT);
  pinMode(btnL, INPUT);
  pinMode(btnG, INPUT);
  pinMode(CANCEL, INPUT);
  
  pinMode(relaySolenoid,  OUTPUT);
  // digitalWrite(relaySolenoid, HIGH);

  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);

  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;

  attachInterrupt(sensorInterrupt, pulseCounter, RISING);

 // Interrupt connected  to PIN D2 executing IncomingImpuls function when signal goes from HIGH to LOW
  attachInterrupt(0,incomingImpuls, FALLING);
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
    delay(100);
    Serial.println("btnML is pressed.");
    dispenseWater(500);
    total_amount = total_amount- 100;
  } 
  
  if (ReadbtnL == HIGH) { //minus 500 pesos
    delay(100);
    Serial.println("btnL is pressed.");
    dispenseWater(1000);
    total_amount = total_amount- 500;
  }  
  
  if (ReadbtnG == HIGH) {// minus 1000 pesos
    delay(100);
    Serial.println("btnG is pressed.");
    dispenseWater(18927);
    total_amount = total_amount- 1000;
  }  
  
  if (ReadCANCEL == HIGH) {
    Serial.println("Cancel is pressed.");
    delay(100);
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
    // ✅ Always check the cancel button immediately
    if (digitalRead(CANCEL) == HIGH) {
      Serial.println("Cancel button pressed. Stopping...");
      digitalWrite(relaySolenoid, LOW);
      delay(100); // Small debounce
      break;
    }

    // Every second, update flow rate and total volume
    if ((millis() - oldTime) > 1000) {
      detachInterrupt(sensorInterrupt);

      // Calculate flow rate and volume
      flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
      oldTime = millis();
      flowMilliLitres = (flowRate * 1000) / 60;
      totalMilliLitres += flowMilliLitres;

      // Print debug info
      Serial.print("Flow rate: ");
      Serial.print(int(flowRate));
      Serial.print(" L/min\t");

      Serial.print("Output Liquid Quantity: ");
      Serial.print(totalMilliLitres);
      Serial.println(" mL");

      // Reset pulse counter and re-enable interrupt
      pulseCount = 0;
      attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
    }
  }

  digitalWrite(relaySolenoid, LOW); // Stop dispensing
  Serial.println("Dispense complete.");
}