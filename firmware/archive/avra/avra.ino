
#define sensor 12
void setup() {
  Serial.begin(9600); 
   pinMode(4, OUTPUT); //switch
  pinMode(sensor, INPUT); 
    //myservo.attach(12);
  Serial.print("Door open/ Close program");
  // put your setup code here, to run once:

}

void loop() {
  
  int detect = digitalRead(sensor);
  
 if(detect==0) 
  {
    digitalWrite(4, HIGH);
    
  }
  else 
  {
    digitalWrite (4, LOW);
  }
}

  
