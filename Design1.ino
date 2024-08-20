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




void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(btnML, INPUT);
  pinMode(btnL, INPUT);
  pinMode(btnG, INPUT);
  pinMode(CANCEL, INPUT);

  pinMode(relaySolenoid,  OUTPUT);
  digitalWrite(relaySolenoid, HIGH);

}

void loop() {
  // put your main code here, to run repeatedly:
  ReadbtnML = digitalRead(btnML);  
  ReadbtnL = digitalRead(btnL);
  ReadbtnG = digitalRead(btnG);
  ReadCANCEL = digitalRead(CANCEL);


  if (ReadbtnML == HIGH) {
    Serial.println("btnML is pressed.");
  } else if (ReadbtnL == HIGH) {
    Serial.println("btnL is pressed.");
  } else if (ReadbtnG == HIGH) {
    Serial.println("btnG is pressed.");
    digitalWrite(relaySolenoid, LOW);
  } else if (ReadCANCEL == HIGH) {
    Serial.println("Cancel is pressed.");
    digitalWrite(relaySolenoid, HIGH);
  }


}
