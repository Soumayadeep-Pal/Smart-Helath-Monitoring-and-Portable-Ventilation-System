#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include <LiquidCrystal.h>
LiquidCrystal lcd(2,3,4,5,6,7);

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

void setup()
{
  Serial.begin(115200); 
  lcd.begin(20,4);

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
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
}

void loop()
{
  lcd.setCursor(0,0);
  lcd.print("Smart Health Monitor");
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

  //calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  //Continuously taking samples from MAX30102.  Heart rate and SpO2 are calculated every 1 second
  while (1)
  {
    //dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
    for (byte i = 25; i < 100; i++)
    {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }

    //take 25 sets of samples before calculating the heart rate.
    for (byte i = 75; i < 100; i++)
    {
      while (particleSensor.available() == false) //do we have new data?
        particleSensor.check(); //Check the sensor for new data

      digitalWrite(readLED, !digitalRead(readLED)); //Blink onboard LED with every data read

      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample(); //We're finished with this sample so move to next sample

      //send samples and calculation result to terminal program through UART
      Serial.print(F("red="));
      Serial.print(redBuffer[i], DEC);
      lcd.setCursor(0,1);
      lcd.print("Heartrate :");
      lcd.print(redBuffer[i]);
      Serial.print(F(", ir="));
      Serial.print(irBuffer[i], DEC);
      lcd.setCursor(0,2);
      lcd.print("Heartrate :");
      lcd.print(irBuffer[i]);

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
}
