/*
 * MAX30102 / MAX30105 SpO2 & Heart Rate Monitor
 * Displays validated BPM and SpO2 readings on a 20x4 LCD and streams telemetry over Serial.
 */

#include <Wire.h>
#include <LiquidCrystal.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

constexpr uint8_t LCD_RS = 2, LCD_EN = 3, LCD_D4 = 4, LCD_D5 = 5, LCD_D6 = 6, LCD_D7 = 7;
constexpr uint8_t PULSE_LED_PIN = 11;
constexpr uint8_t READ_LED_PIN = 13;

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
MAX30105 particleSensor;

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
uint16_t irBuffer[100];
uint16_t redBuffer[100];
#else
uint32_t irBuffer[100];
uint32_t redBuffer[100];
#endif

int32_t bufferLength = 100;
int32_t spo2 = 0;
int8_t validSPO2 = 0;
int32_t heartRate = 0;
int8_t validHeartRate = 0;

void setup() {
  Serial.begin(115200);

  lcd.begin(20, 4);
  lcd.setCursor(0, 0);
  lcd.print("SMART HEALTH MONITOR");
  lcd.setCursor(0, 3);
  lcd.print("    HEALTH MATE     ");

  pinMode(PULSE_LED_PIN, OUTPUT);
  pinMode(READ_LED_PIN, OUTPUT);

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println(F("MAX30105/MAX30102 not found. Check wiring/power."));
    lcd.setCursor(0, 1);
    lcd.print("Sensor Not Found!");
    while (1);
  }

  byte ledBrightness = 60;
  byte sampleAverage = 4;
  byte ledMode = 2;       // Red + IR
  byte sampleRate = 100;
  int pulseWidth = 411;
  int adcRange = 4096;

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
}

void loop() {
  // Initial 100-sample buffer
  for (byte i = 0; i < bufferLength; i++) {
    while (!particleSensor.available()) {
      particleSensor.check();
    }
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
  }

  maxim_heart_rate_and_oxygen_saturation(
      irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  while (true) {
    for (byte i = 25; i < 100; i++) {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }

    for (byte i = 75; i < 100; i++) {
      while (!particleSensor.available()) {
        particleSensor.check();
      }
      digitalWrite(READ_LED_PIN, !digitalRead(READ_LED_PIN));

      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample();
    }

    maxim_heart_rate_and_oxygen_saturation(
        irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

    Serial.print(F("HR="));
    Serial.print(heartRate, DEC);
    Serial.print(F(", HRvalid="));
    Serial.print(validHeartRate, DEC);
    Serial.print(F(", SPO2="));
    Serial.print(spo2, DEC);
    Serial.print(F(", SPO2Valid="));
    Serial.println(validSPO2, DEC);

    lcd.setCursor(0, 1);
    if (validHeartRate == 1) {
      lcd.print("HeartRate: ");
      lcd.print(heartRate);
      lcd.print(" BPM   ");
    } else {
      lcd.print("HeartRate: -- BPM   ");
    }

    lcd.setCursor(0, 2);
    if (validSPO2 == 1) {
      lcd.print("SPO2:      ");
      lcd.print(spo2);
      lcd.print(" %     ");
    } else {
      lcd.print("SPO2:      -- %     ");
    }
  }
}
