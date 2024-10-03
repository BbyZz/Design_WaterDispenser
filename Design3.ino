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
byte sensorInterrupt = 0;  // 0 = digital pin 2
byte sensorPin       = 2;


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
  digitalWrite(relaySolenoid, HIGH);

  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);

  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;

  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);

 // Interrupt connected  to PIN D2 executing IncomingImpuls function when signal goes from HIGH to LOW
  attachInterrupt(1,incomingImpuls, FALLING);
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

  if (ReadbtnML == HIGH) {
    delay(100);
    Serial.println("btnML is pressed.");
    dispenseWater(500);
  } 
  
  if (ReadbtnL == HIGH) {
    delay(100);
    Serial.println("btnL is pressed.");
    dispenseWater(1000);
  }  
  
  if (ReadbtnG == HIGH) {
    delay(100);
    Serial.println("btnG is pressed.");
    dispenseWater(18927);
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

void dispenseWater(float totalVolume){
  ReadCANCEL = digitalRead(CANCEL);
  digitalWrite(relaySolenoid, HIGH);
  Serial.print("Dispensing: ");
  Serial.println(totalVolume); 

  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("Dispensing: ");
  lcd.setCursor(1,1);
  lcd.print(totalVolume);

  while (totalVolume > totalMilliLitres && ReadCANCEL == LOW) {
      if((millis() - oldTime) > 1000)  { // Only process counters once per second  

      detachInterrupt(sensorInterrupt);
          
      
      flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
      
      
      oldTime = millis();
      

      flowMilliLitres = (flowRate / 60) * 1000;
      
  
      totalMilliLitres += flowMilliLitres;
        
      unsigned int frac;

      // Print the flow rate for this second in litres / minute
      Serial.print("Flow rate: ");
      Serial.print(int(flowRate));  // Print the integer part of the variable
      Serial.print("L/min");
      Serial.print("\t"); 		  // Print tab space

      // Print the cumulative total of litres flowed since starting
      Serial.print("Output Liquid Quantity: ");        
      Serial.print(totalMilliLitres);
      Serial.println("mL"); 
      Serial.print("\t"); 		  // Print tab space
      Serial.print(totalMilliLitres/1000);
      Serial.print("L");
      
      // Reset the pulse counter so we can start incrementing again
      pulseCount = 0;
      
      // Enable the interrupt again now that we've finished sending output
      attachInterrupt(sensorInterrupt, pulseCounter, FALLING);

      if (ReadCANCEL == HIGH) {
      Serial.println("Cancel is pressed.");
      delay(100);
      digitalWrite(relaySolenoid, LOW);
      break;
      }
    } else if (ReadCANCEL == HIGH) {
      Serial.println("Cancel is pressed.");
      delay(100);
      digitalWrite(relaySolenoid, LOW);
      break;
    }
  }
  totalMilliLitres = 0;
}


