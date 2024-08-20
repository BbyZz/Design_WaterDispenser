// //components
// lcd = SCL AND SDA / PC5 PC4
// relayCoin = 9 
// coinSignal =  to digital pin 6 ---  2
int relaySolenoid =  12;
// waterFlowSensor = 13
int btnML = 3;
int btnL = 4;
int btnG = 5;
int CANCEL = 6;

int ReadbtnML  = 0;
int ReadbtnL   = 0;
int ReadbtnG   = 0;
int ReadCANCEL = 0; 

// esp8266 ml -  out 7
// esp8266 L - out  11
// esp8266 G - out  10


// PLAN TO MOVE COIN SIGNAL TO A PWM ~

byte sensorInterrupt = 0;  // 0 = digital pin 2
byte sensorPin       = 2;


float calibrationFactor = 4.5;

volatile byte pulseCount;  

float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

unsigned long oldTime;




void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

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

}

void loop() {
  // put your main code here, to run repeatedly:
  ReadbtnML = digitalRead(btnML);  
  ReadbtnL = digitalRead(btnL);
  ReadbtnG = digitalRead(btnG);
  ReadCANCEL = digitalRead(CANCEL);


  if (ReadbtnML == HIGH) {
    delay(100);
    Serial.println("btnML is pressed.");
    dispenseWater(500);
  } else if (ReadbtnL == HIGH) {
    delay(100);
    Serial.println("btnL is pressed.");
    dispenseWater(1000);
  } else if (ReadbtnG == HIGH) {
    delay(100);
    Serial.println("btnG is pressed.");
    dispenseWater(18927);
  } else if (ReadCANCEL == HIGH) {
    Serial.println("Cancel is pressed.");
    delay(100);
    digitalWrite(relaySolenoid, LOW);
  }


}

void dispenseWater(float totalVolume){
  digitalWrite(relaySolenoid, HIGH);
  Serial.print("Dispensing: ");
  Serial.println(totalVolume); 

  while (totalVolume > totalMilliLitres) {
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
    }
  }
  totalMilliLitres = 0;
}

void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}
