#include <Servo.h>
#include <DHT.h>;
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

MAX30105 particleSensor;

#define MAX_BRIGHTNESS 255

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)

uint16_t irBuffer[100]; 
uint16_t redBuffer[100];  
#else
uint32_t irBuffer[100]; 
uint32_t redBuffer[100];  
#endif

int32_t bufferLength; 
int32_t spo2; 
int8_t validSPO2; 
int32_t heartRate; 
int8_t validHeartRate;

byte pulseLED = 11; 
byte readLED = 13;


LiquidCrystal_I2C lcd(0x27,16,2);

#define ONE_WIRE_BUS 2
#define DHTPIN 7    
#define DHTTYPE DHT11  
DHT dht(DHTPIN, DHTTYPE);

int h;  
int t;
int p;

Servo myservo;
int pos = 0;  
int sen = A0;
float val;
float ven;

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

int C = 0;
float F = 0;



void setup()
{
  Serial.begin(115200);
  pinMode(pulseLED, OUTPUT);
  pinMode(readLED, OUTPUT);
    if (!particleSensor.begin(Wire, I2C_SPEED_FAST))
  {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while (1);
  }

  Serial.println(F("Attach sensor to finger with rubber band. Press any key to start conversion"));
  while (Serial.available() == 0) ; 
  Serial.read();

  byte ledBrightness = 60;
  byte sampleAverage = 4; 
  byte ledMode = 2;
  byte sampleRate = 100;
  int pulseWidth = 411;
  int adcRange = 4096; 

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings

myservo.attach(9);
  dht.begin();
  lcd.begin(20,4,LCD_5x8DOTS);
  lcd.backlight();
  }


  
void loop(){
  bufferLength = 100; 

  for (byte i = 0 ; i < bufferLength ; i++)
  {
    while (particleSensor.available() == false) 
      particleSensor.check();

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample(); 

    Serial.print(F("red="));
    Serial.print(redBuffer[i], DEC);
    Serial.print(F(", ir="));
    Serial.println(irBuffer[i], DEC);
  }

  
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  while (1)
  {
    for (byte i = 25; i < 100; i++)
    {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }
    for (byte i = 75; i < 100; i++)
    {
      while (particleSensor.available() == false) 
        particleSensor.check();

      digitalWrite(readLED, !digitalRead(readLED)); 

      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample(); 

      Serial.print(F("red="));
      Serial.print(redBuffer[i], DEC);
      Serial.print(F(", ir="));
      Serial.print(irBuffer[i], DEC);

      Serial.print(F(", HR="));
      Serial.print(heartRate, DEC);

      Serial.print(F(", HRvalid="));
      Serial.print(validHeartRate, DEC);

      Serial.print(F(", SPO2="));
      Serial.print(spo2, DEC);

      Serial.print(F(", SPO2Valid="));
      Serial.println(validSPO2, DEC);
    }
 
   maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  }
   {
    val = analogRead(sen);
    Serial.println(val);
    Serial.println(t);
    Serial.println(h);
   
    if (val>0 && val<=250)
  {
      for (pos = 0; pos <= 120; pos += 1)
    {
      myservo.write(pos);          
      delay(20);
    }                      
      for (pos = 120; pos >= 0; pos -= 1)
    {
      myservo.write(pos);            
      delay(8);      
    }
  }
      else if (val>250 && val<=500)
  {
      for (pos = 0; pos <= 120; pos += 1)
    {
      myservo.write(pos);          
      delay(18);
    }                      
      for (pos = 120; pos >= 0; pos -= 1)
    {
      myservo.write(pos);            
      delay(8);      
    }
  }
        else if (val>500 && val<=700)
  {
      for (pos = 0; pos <= 120; pos += 1)
    {
      myservo.write(pos);          
      delay(16);
    }                      
      for (pos = 120; pos >= 0; pos -= 1)
    {
      myservo.write(pos);            
      delay(8);      
    }
  }
        else if (val>700 && val<=900)
  {
      for (pos = 0; pos <= 120; pos += 1)
    {
      myservo.write(pos);          
      delay(14);
    }                      
      for (pos = 120; pos >= 0; pos -= 1)
    {
      myservo.write(pos);            
      delay(8);      
    }
  }
          else if (val>700 && val<=900)
  {
      for (pos = 0; pos <= 120; pos += 1)
    {
      myservo.write(pos);          
      delay(12);
    }                      
      for (pos = 120; pos >= 0; pos -= 1)
    {
      myservo.write(pos);            
      delay(8);      
    }
  }
          else if (val>900)
  {
      for (pos = 0; pos <= 120; pos += 1)
    {
      myservo.write(pos);          
      delay(10);
    }                      
      for (pos = 120; pos >= 0; pos -= 1)
    {
      myservo.write(pos);            
      delay(8);      
    }
  }
  sensors.requestTemperatures();

  C = sensors.getTempCByIndex(0);
  F = sensors.toFahrenheit(C);
  ven = (val/1000)*100;

  Serial.print(C);
  Serial.print(" C  ");
  Serial.print(F);
  Serial.println(" F");
  
    h = dht.readHumidity();
    //t = dht.readTemperature();
 
    lcd.setCursor(0,0);
    lcd.println("SMART HEALTH MONITOR");
    //lcd.setCursor(0, 1);
    //lcd.print("TEMP:");
    //lcd.print(t);
    //lcd.print("C");
     
    lcd.setCursor(0,1);
    lcd.print("HUMI:");
    lcd.print(h);
    lcd.print("%");

    lcd.setCursor(9,1);
    lcd.print("VEN:");
    lcd.print(ven);
     lcd.print("% ");
    lcd.setCursor(0,2);
    lcd.print("TEMP:");
    lcd.print(C);
    lcd.print("C & ");
    lcd.print(F);
    lcd.print("F");
          if(validSPO2==1)
          {
        lcd.setCursor(0,3);
        lcd.print("SPO2:");
        lcd.print(spo2,DEC);
        lcd.print("%");
          }
       
          if (validHeartRate==1)
          {
        lcd.setCursor(12,3);
        lcd.print("HR");
        lcd.print(heartRate,DEC);
          }
}
}
