#define TOP_BTN_PIN 5
#define BOTTOM_BTN_PIN 4
#define ROTARY_ENCODER_BTN_PIN 9
bool topBtn() { return digitalRead(TOP_BTN_PIN) == 1; }
bool bottomBtn() { return digitalRead(BOTTOM_BTN_PIN) == 1; }
bool middleBtn() { return digitalRead(ROTARY_ENCODER_BTN_PIN) == 0; }


#define CHARGE_PIN 0
bool isCharging() { digitalRead(CHARGE_PIN) == 0; }


#define BUZZER_PIN 3
void beep() { tone(BUZZER_PIN, 1000, 25); }


// https://github.com/mathertel/RotaryEncoder
#include <RotaryEncoder.h>
#define ROTARY_ENCODER_A_PIN 2
#define ROTARY_ENCODER_B_PIN 8
RotaryEncoder encoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, RotaryEncoder::LatchMode::FOUR3);


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



#define BATTERY_READ_PIN 1
#define ADC_R1 33000.0
#define ADC_R2 47000.0
#define FULL_VOLTAGE 4.2
#define EMPTY_VOLTAGE 3.3
//#define ADC_READINGS 10
//#define ADC_REF_VOLTAGE 3.3
/*uint16_t readBatteryADC()
{
  uint32_t avg = 0;
  for (int i = 0; i < ADC_READINGS; i++)
  {
    avg += analogRead(BATTERY_READ_PIN);
    delay(2);
  }
  avg /= ADC_READINGS;
  return avg;
}
double readBatteryVoltageADC() 
{
  double Vout = ((double)readBatteryADC() / 4095.0) * ADC_REF_VOLTAGE;
  double Vbat = Vout * ((ADC_R1 + ADC_R2) / ADC_R2);
  return Vbat;
}
*/
double readBatteryVoltage() 
{
  double Vbat = ((double)analogReadMilliVolts(BATTERY_READ_PIN) / 1000.0) * ((ADC_R1 + ADC_R2) / ADC_R2);
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