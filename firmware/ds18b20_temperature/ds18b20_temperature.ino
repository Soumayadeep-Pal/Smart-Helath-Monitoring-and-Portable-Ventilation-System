/*
 * DS18B20 Body Temperature Monitor with Fever Alert (LED + Buzzer)
 */

#include <LiquidCrystal.h>

// Pin Definitions
constexpr uint8_t LCD_RS = 2, LCD_EN = 3, LCD_D4 = 4, LCD_D5 = 5, LCD_D6 = 6, LCD_D7 = 7;
constexpr uint8_t DS18B20_PIN = A0;
constexpr uint8_t GREEN_LED_PIN = 8;
constexpr uint8_t RED_LED_PIN = 9;
constexpr uint8_t BUZZER_PIN = 13;

constexpr float FEVER_THRESHOLD_F = 100.0f;

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

bool ds18b20Start() {
  bool present = false;
  digitalWrite(DS18B20_PIN, LOW);
  pinMode(DS18B20_PIN, OUTPUT);
  delayMicroseconds(500);
  pinMode(DS18B20_PIN, INPUT);
  delayMicroseconds(100);
  if (!digitalRead(DS18B20_PIN)) {
    present = true;
    delayMicroseconds(400);
  }
  return present;
}

void ds18b20WriteBit(bool value) {
  digitalWrite(DS18B20_PIN, LOW);
  pinMode(DS18B20_PIN, OUTPUT);
  delayMicroseconds(2);
  digitalWrite(DS18B20_PIN, value);
  delayMicroseconds(80);
  pinMode(DS18B20_PIN, INPUT);
  delayMicroseconds(2);
}

void ds18b20WriteByte(byte value) {
  for (byte i = 0; i < 8; i++) {
    ds18b20WriteBit(bitRead(value, i));
  }
}

bool ds18b20ReadBit() {
  digitalWrite(DS18B20_PIN, LOW);
  pinMode(DS18B20_PIN, OUTPUT);
  delayMicroseconds(2);
  pinMode(DS18B20_PIN, INPUT);
  delayMicroseconds(5);
  bool value = digitalRead(DS18B20_PIN);
  delayMicroseconds(100);
  return value;
}

byte ds18b20ReadByte() {
  byte value = 0;
  for (byte i = 0; i < 8; i++) {
    bitWrite(value, i, ds18b20ReadBit());
  }
  return value;
}

bool ds18b20Read(int *rawTempValue) {
  if (!ds18b20Start()) return false;
  ds18b20WriteByte(0xCC); // Skip ROM
  ds18b20WriteByte(0x44); // Convert T
  while (ds18b20ReadByte() == 0);

  if (!ds18b20Start()) return false;
  ds18b20WriteByte(0xCC); // Skip ROM
  ds18b20WriteByte(0xBE); // Read Scratchpad
  *rawTempValue = ds18b20ReadByte();
  *rawTempValue |= (unsigned int)(ds18b20ReadByte() << 8);
  return true;
}

void setup() {
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  Health Mate   ");
  lcd.setCursor(0, 1);
  lcd.print(" DS18B20 Sensor ");
  delay(1500);
  lcd.clear();
  Serial.begin(9600);
}

void loop() {
  int rawTemp = 0;
  lcd.setCursor(0, 0);
  lcd.print("  Temperature   ");

  if (ds18b20Read(&rawTemp)) {
    float tempC = (float)rawTemp / 16.0f;
    float tempF = (tempC * 9.0f) / 5.0f + 32.0f;

    Serial.print("Temperature: ");
    Serial.print(tempC, 2);
    Serial.print(" C | ");
    Serial.print(tempF, 2);
    Serial.println(" F");

    lcd.setCursor(0, 1);
    lcd.print(tempC, 1);
    lcd.print("C  ");
    lcd.setCursor(8, 1);
    lcd.print(tempF, 1);
    lcd.print("F  ");

    if (tempF > FEVER_THRESHOLD_F) {
      digitalWrite(BUZZER_PIN, HIGH);
      digitalWrite(GREEN_LED_PIN, LOW);
      digitalWrite(RED_LED_PIN, HIGH);
      delay(300);
      digitalWrite(BUZZER_PIN, LOW);
    } else {
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(GREEN_LED_PIN, HIGH);
      digitalWrite(RED_LED_PIN, LOW);
    }
  } else {
    lcd.setCursor(0, 1);
    lcd.print("  Sensor Error  ");
  }

  delay(500);
}
