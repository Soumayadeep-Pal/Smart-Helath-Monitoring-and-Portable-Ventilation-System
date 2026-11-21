#include <Servo.h>
Servo myservo;
int pos = 0;
void setup() {
  Serial.begin(9600); 
  pinMode(2, OUTPUT); //led
  pinMode(9, OUTPUT); //trig
  pinMode(10, INPUT);//echo
    myservo.attach(12);
  Serial.print("Door open/ Close program");
  // put your setup code here, to run once:

}
void loop() {
  long duration, distance;
  digitalWrite(9,HIGH);
  delayMicroseconds(1000);
  digitalWrite(9, LOW);
  duration=pulseIn(10, HIGH);
  distance =(duration/2)/29.1;
  Serial.print(distance);
  Serial.println("CM");
  delay(10);
 
 if((distance<=30)) 
  {
    digitalWrite(2, HIGH);
    for (pos = 0; pos<=180; pos+=1)
    {
      myservo.write(pos);
      delay(15);
    }
  }
 else
 {
     digitalWrite(2, LOW);
    
   }
}
