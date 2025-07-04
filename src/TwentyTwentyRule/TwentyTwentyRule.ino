#include "preferences.h"

#include <DonutStudioTimer.h>
Timer countdown = Timer();

//IRAM_ATTR void checkPosition() { encoder.tick(); }


bool isMinuteTimer = true;


void setup() 
{
  // --- INITIALIZE PINS ---
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SLIDER_PIN, INPUT);
  analogSetAttenuation(ADC_2_5db);
  pinMode(SIDE_BTN_PIN, INPUT);
  pinMode(ROTARY_ENC_BTN_PIN, INPUT);

  // --- ATTATCH INTERRUPTS FOR ROTARY ENCODER ---
  //attachInterrupt(digitalPinToInterrupt(ROTARY_ENC_A_PIN), checkPosition, CHANGE);
  //attachInterrupt(digitalPinToInterrupt(ROTARY_ENC_B_PIN), checkPosition, CHANGE);

  // --- INITIALIZE LED RING ---
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, LED_COUNT).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(30);
  
  // --- INITIALIZE DISPLAY ---
  u8x8.begin();
  u8x8.setFont(u8x8_font_pressstart2p_f);

  // --- INITIALIZE EEPROM AND LOAD SLIDER VALUES ---
  EEPROM.begin(4);
  delay(250);
  loadSliderValues();

  countdown.setTimer(0, 20);
  countdown.begin();
  setDisplay();
}
void loop()
{
  if (middleBtn())
  {
    playSound();
    while (middleBtn()) { delay(2); }

    countdown.setPause(!countdown.isPaused());
    if (countdown.isPaused()) 
    {
      fill_solid(leds, LED_COUNT, CRGB::Black);
      FastLED.show();
    }
  }

  if (rightBtn()) 
  {
    playSound();
    while (rightBtn()) { delay(2); }

    skipToNext(false);
  }

  if (countdown.isOver())
    skipToNext(true);

  if (!countdown.isPaused())
    displayLeds(countdown, isMinuteTimer ? CRGB::Red : CRGB::Green);
  else
  {
    EVERY_N_MILLISECONDS(25) 
    {
      fadeToBlackBy(leds, LED_COUNT, 32); // 32/255 = 13%
      FastLED.show();
    }
    EVERY_N_MILLISECONDS(250) 
    {
      leds[random8(LED_COUNT)] = CRGB::Yellow;
      FastLED.show();
    }
  }

  displayTimer(countdown);
}

void skipToNext(bool alarm)
{
  if (alarm)
  {
    u8x8.clear();
    u8x8.draw2x2String(2, 2, "ENDED!");

    fill_solid(leds, LED_COUNT, CRGB::Black);
    FastLED.show();

    unsigned long t = millis();
    uint8_t i = 0;
    uint8_t half = (LED_COUNT / 2);
    while (millis() - t < 1500)
    {
      EVERY_N_MILLISECONDS(250) { playSound(); }
      EVERY_N_MILLISECONDS(25)
      {
        i = (i + 1) % half;
        leds[i] = CRGB::Red;
        leds[i + half] = CRGB::Red;
        fadeToBlackBy(leds, LED_COUNT, 64); // 64/255 = 25%
        FastLED.show(); 
      }

      if (anyBtn())
      {
        playSound();
        while (anyBtn()) { delay(2); }
        break;
      }
      
      delay(2);
    }
  }

  isMinuteTimer = !isMinuteTimer;
  if (isMinuteTimer)
    countdown.setTimer(0, 20, 10); // 10 seconds additional time
  else
    countdown.setTimer(0, 0, 30); // 10 seconds additional time
  countdown.begin();
  setDisplay();
}

void displayLeds(Timer& t, CRGB color)
{
  EVERY_N_MILLISECONDS(25) 
  {
    float percent = (float)t.getTotalRemainingMilliseconds() / t.getTotalMilliseconds();
    int count = percent * LED_COUNT;
    for (int i = 0; i < LED_COUNT; i++)
    {
      if (i < count)
        leds[i] = color;
      else if (i == count) 
      {
        float n = percent * LED_COUNT - count;
        leds[i] = color;
        leds[i].fadeToBlackBy(255 - (uint8_t)(n * 255));
      }
      else
        leds[i] = CRGB::Black;
    }
    FastLED.show();
  }
}
void displayTimer(Timer& t) 
{
  uint8_t s = t.getRemainingSeconds();
  uint8_t m = t.getRemainingMinutes();
  uint8_t h = t.getRemainingHours();

  String text = "";
  if (h < 10)
    text += "0";
  text += String(h) + ":";
  if (m < 10)
    text += "0";
  text += String(m) + ":";
  if (s < 10)
    text += "0";
  text += String(s);

  u8x8.draw2x2String(0, 2, text.c_str());
}
void setDisplay()
{
  u8x8.clearLine(0);
  if (isMinuteTimer)
    u8x8.drawString(3, 0, "Work Time!");
  else
    u8x8.drawString(3, 0, "Break Time!");
}