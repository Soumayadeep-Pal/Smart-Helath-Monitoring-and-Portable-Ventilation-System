/*
 * Smart Health Monitor & Portable Ventilation Controller
 * Sensors: DS18B20 (Temperature), DHT11 (Ambient Humidity/Temp), Analog Ventilation Control Potentiometer
 * Actuator: Servo-driven Ambu-bag Ventilation Mechanism
 * Display: 20x4 I2C LCD (0x27)
 */

#include <Wire.h>
#include <Servo.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>

// Pin Definitions
constexpr uint8_t ONE_WIRE_BUS_PIN = 2;
constexpr uint8_t DHT_PIN = 7;
constexpr uint8_t SERVO_PIN = 9;
constexpr uint8_t VENT_POT_PIN = A0;

// Sensor & Display Setup
constexpr uint8_t DHT_TYPE = DHT11;
constexpr uint8_t LCD_I2C_ADDR = 0x27;
constexpr uint8_t LCD_COLS = 20;
constexpr uint8_t LCD_ROWS = 4;

DHT dht(DHT_PIN, DHT_TYPE);
OneWire oneWire(ONE_WIRE_BUS_PIN);
DallasTemperature tempSensors(&oneWire);
LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);
Servo ventServo;

void sweepVentilationServo(uint8_t stepDelayMs) {
  for (int pos = 0; pos <= 120; pos++) {
    ventServo.write(pos);
    delay(stepDelayMs);
  }
  for (int pos = 120; pos >= 0; pos--) {
    ventServo.write(pos);
    delay(8);
  }
}

uint8_t getVentilationStepDelay(int analogVal) {
  if (analogVal <= 250) return 20;
  if (analogVal <= 500) return 18;
  if (analogVal <= 700) return 16;
  if (analogVal <= 850) return 14;
  if (analogVal <= 950) return 12;
  return 10;
}

void setup() {
  Serial.begin(9600);
  ventServo.attach(SERVO_PIN);
  dht.begin();
  tempSensors.begin();

  lcd.begin(LCD_COLS, LCD_ROWS, LCD_5x8DOTS);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("SMART HEALTH MONITOR");
}

void loop() {
  int potRaw = analogRead(VENT_POT_PIN);
  float ventPercent = (potRaw / 1023.0f) * 100.0f;

  if (potRaw > 0) {
    uint8_t stepDelay = getVentilationStepDelay(potRaw);
    sweepVentilationServo(stepDelay);
  }

  tempSensors.requestTemperatures();
  float bodyTempC = tempSensors.getTempCByIndex(0);
  float bodyTempF = tempSensors.toFahrenheit(bodyTempC);

  float humidity = dht.readHumidity();
  float ambientTempC = dht.readTemperature();

  Serial.print("POT=");
  Serial.print(potRaw);
  Serial.print(" | VEN=");
  Serial.print(ventPercent, 1);
  Serial.print("% | BodyTemp=");
  Serial.print(bodyTempC, 1);
  Serial.print("C | Ambient=");
  Serial.print(ambientTempC, 1);
  Serial.print("C | Humidity=");
  Serial.print(humidity, 1);
  Serial.println("%");

  lcd.setCursor(0, 0);
  lcd.print("SMART HEALTH MONITOR");

  lcd.setCursor(0, 1);
  lcd.print("HUMI:");
  lcd.print((int)humidity);
  lcd.print("%  ");

  lcd.setCursor(9, 1);
  lcd.print("VEN:");
  lcd.print((int)ventPercent);
  lcd.print("%  ");

  lcd.setCursor(0, 2);
  lcd.print("TEMP:");
  lcd.print(bodyTempC, 1);
  lcd.print("C/");
  lcd.print(bodyTempF, 1);
  lcd.print("F ");
}
