#include <Servo.h>
#include <DHT.h>;
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>

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
  myservo.attach(9);
  dht.begin();
  lcd.begin(20,4,LCD_5x8DOTS);
  lcd.backlight();
  Serial.begin(9600);
}

void loop()
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
    t = dht.readTemperature();
 {
    lcd.setCursor(0,0);
    lcd.println("SMART HEALTH MONITOR");
    
 }
   // lcd.setCursor(0, 1);
    //lcd.print("TEMP:");
   // lcd.print(t);
    //lcd.print("C");
     
    lcd.setCursor(0,1);
    lcd.print("HUMI:");
    lcd.print(h);
    lcd.print("%");
    Serial.println(h);

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
    
}
