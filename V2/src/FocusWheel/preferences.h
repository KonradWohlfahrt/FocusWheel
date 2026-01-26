#include <EEPROM.h>


#define TOP_BTN_PIN 4
#define BOTTOM_BTN_PIN 5
#define ROTARY_ENCODER_BTN_PIN 9
bool topBtn() { return digitalRead(TOP_BTN_PIN) == 1; }
bool bottomBtn() { return digitalRead(BOTTOM_BTN_PIN) == 1; }
bool middleBtn() { return digitalRead(ROTARY_ENCODER_BTN_PIN) == 0; }


#include "pitches.h"
#define BUZZER_PIN 3
#define MELODY_LENGTH 5
#define MELODYS 6
uint16_t melodys[MELODYS][MELODY_LENGTH] PROGMEM = {
  { NOTE_C4, NOTE_E4, NOTE_G4, NOTE_E4, NOTE_C4 },
  { NOTE_G3, NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5 },
  { NOTE_E4, NOTE_G4, NOTE_E5, NOTE_G4, NOTE_E4 },
  { NOTE_G4, NOTE_E4, NOTE_C4, NOTE_G3, NOTE_C4 },
  { NOTE_C5, NOTE_E5, NOTE_G5, NOTE_E5, NOTE_C5 },
  { NOTE_AS3, NOTE_C4, NOTE_DS4, NOTE_C4, NOTE_AS3 }
  // { NOTE_, NOTE_, NOTE_, NOTE_, NOTE_ }
};
uint8_t durations[MELODYS][MELODY_LENGTH] PROGMEM = {
  { 16, 8, 8, 16, 4 },
  { 16, 8, 8, 8, 4 },
  { 8, 16, 8, 16, 4 },
  { 16, 8, 8, 16, 4 },
  { 16, 16, 8, 8, 4 },
  { 16, 8, 8, 16, 4 }
  // { , , , ,  }
};
void beep(uint16_t freq, uint16_t time) { tone(BUZZER_PIN, freq, time); }
void beep(uint16_t time) { tone(BUZZER_PIN, 1000, time); }
void beep() { beep(25); }
void doubleBeep() { beep(300, 50); delay(100); beep(300, 50); }
uint16_t noteDuration(uint8_t dur) { return 1000 / (uint16_t)dur; }
void playMelody(uint16_t mel[], uint8_t dur[]) 
{
  for (uint8_t i = 0; i < MELODY_LENGTH; i++)
  {
    beep(mel[i], noteDuration(dur[i]));
    delay(noteDuration(dur[i]) * 1.3);
  }
}


// https://github.com/mathertel/RotaryEncoder
#include <RotaryEncoder.h>
#define ROTARY_ENCODER_A_PIN 2
#define ROTARY_ENCODER_B_PIN 8
RotaryEncoder encoder(ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_A_PIN, RotaryEncoder::LatchMode::FOUR3);


// https://github.com/FastLED/FastLED
#include <FastLED.h>
#define LED_PIN 10
#define LED_COUNT 16
CRGB leds[LED_COUNT];


// https://github.com/olikraus/u8g2
#include <U8x8lib.h>
#define SDA_PIN 6
#define SCL_PIN 7
U8X8_SSD1306_128X32_UNIVISION_SW_I2C u8x8(SCL_PIN, SDA_PIN, U8X8_PIN_NONE);


// NOTE: resistor in kOhm
#define ADC_R1 33.0
#define ADC_R2 47.0
#define FULL_VOLTAGE 4.2
#define EMPTY_VOLTAGE 3.3
#define BATTERY_READ_PIN 1
double readBatteryVoltage() 
{
  double Vbat = (analogReadMilliVolts(BATTERY_READ_PIN) / 1000.0) * ((ADC_R1 + ADC_R2) / ADC_R2);
  return Vbat;
}
bool isBatteryPowered() 
{
  return readBatteryVoltage() <= 4.3;
}
uint8_t getBatteryLevel()
{
  double Vbat = constrain(readBatteryVoltage(), EMPTY_VOLTAGE, FULL_VOLTAGE);
  return (uint8_t)(((Vbat - EMPTY_VOLTAGE) / (FULL_VOLTAGE - EMPTY_VOLTAGE)) * 100.0);
}

#define CHARGE_PIN 0
bool isCharging() { return readBatteryVoltage() > 4.3; /*return digitalRead(CHARGE_PIN) == 0;*/ }


/*
  --- UTILITY METHODS ---
*/
bool isInterval(uint16_t interval) { return millis() % interval > (interval / 2); }
bool isTimestamp(unsigned long& timestamp, uint16_t interval) 
{
  if (millis() - timestamp >= interval)
  {
    timestamp = millis();
    return true;
  }
  return false;
}