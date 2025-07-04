#include <EEPROM.h>


#define SIDE_BTN_PIN 1
#define ROTARY_ENC_BTN_PIN 9
bool rightBtn() { return digitalRead(SIDE_BTN_PIN) == 0; }
bool middleBtn() { return digitalRead(ROTARY_ENC_BTN_PIN) == 0; }
bool anyBtn() { return rightBtn() || middleBtn(); }


#include <RotaryEncoder.h>
#define ROTARY_ENC_A_PIN 2
#define ROTARY_ENC_B_PIN 8
RotaryEncoder encoder(ROTARY_ENC_A_PIN, ROTARY_ENC_B_PIN, RotaryEncoder::LatchMode::FOUR3);


#include <FastLED.h>
#define LED_PIN 10
#define LED_COUNT 16
CRGB leds[LED_COUNT];


#include <U8x8lib.h>
#define SCL 7
#define SDA 6
U8X8_SSD1306_128X32_UNIVISION_SW_I2C u8x8(/* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);


#define ADC_READINGS 10
#define SLIDER_PIN 4
uint16_t _sliderMin = 0;
uint16_t _sliderMax = 4095;
uint16_t readSlider()
{
  uint32_t avg = 0;
  for (int i = 0; i < ADC_READINGS; i++)
  {
    avg += analogRead(SLIDER_PIN);
    delay(1);
  }
  avg /= ADC_READINGS;
  return avg;
}
uint8_t getSliderPosition()
{
  uint16_t slider = readSlider();
  slider = constrain(slider, _sliderMin, _sliderMax);
  return map(slider, _sliderMin, _sliderMax, 0, 255);
}
void loadSliderValues()
{
  _sliderMin = EEPROM.readUInt(0);
  _sliderMax = EEPROM.readUInt(2);
}


#define BUZZER_PIN 3
void playSound() { analogWrite(BUZZER_PIN, getSliderPosition()); delay(25); analogWrite(BUZZER_PIN, 0); }


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