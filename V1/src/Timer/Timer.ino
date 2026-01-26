#include "preferences.h"

#include <DonutStudioTimer.h>
Timer countdown = Timer(); 

unsigned long timestamp;
bool countdownStarted = false;


IRAM_ATTR void checkPosition() { encoder.tick(); }


void setup() 
{
  // --- INITIALIZE PINS ---
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SLIDER_PIN, INPUT);
  analogSetAttenuation(ADC_2_5db);
  pinMode(SIDE_BTN_PIN, INPUT);
  pinMode(ROTARY_ENC_BTN_PIN, INPUT);

  // --- ATTATCH INTERRUPTS FOR ROTARY ENCODER ---
  attachInterrupt(digitalPinToInterrupt(ROTARY_ENC_A_PIN), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ROTARY_ENC_B_PIN), checkPosition, CHANGE);

  // --- INITIALIZE LED RING ---
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, LED_COUNT).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(30);
  
  // --- INITIALIZE DISPLAY ---
  u8x8.begin();
  u8x8.setFont(u8x8_font_pressstart2p_f);

  // --- INITIALIZE EEPROM AND LOAD SLIDER VALUES ---
  EEPROM.begin(7);
  delay(250);
  loadSliderValues();
  uint8_t s = constrain(EEPROM.read(4), 0, 59);
  uint8_t m = constrain(EEPROM.read(5), 0, 59);
  uint8_t h = constrain(EEPROM.read(6), 0, 3);
  countdown.setTimer(h, m, s);

  setDisplay();
}
void loop()
{
  if (middleBtn())
  {
    playSound();
    while (middleBtn()) 
    {
      refreshDevice();
      delay(2);
    }

    if (!countdownStarted)
    {
      countdownStarted = true;
      countdown.begin();
      u8x8.clear();
    }
    else 
    {
      countdown.setPause(!countdown.isPaused());
      if (countdown.isPaused()) 
      {
        fill_solid(leds, LED_COUNT, CRGB::Black);
        FastLED.show();
      }
    }
  }

  if (rightBtn())
  {
    playSound();
    unsigned long t = millis();
    bool longPress = false;
    while (rightBtn())
    {
      if (!longPress && millis() - t > 700)
      {
        longPress = true;
        playSound();delay(20);playSound();
      }
      refreshDevice();
      delay(2);
    }

    if (countdownStarted)
    {
      countdownStarted = false;
      setDisplay();
    }
    else
    {
      if (!longPress)
        editTimer();
      else
      {
        u8x8.clear();
        u8x8.draw2x2String(2, 2, "SAVED!");
        for (int i = 0; i < 3; i++)
        {
          fill_solid(leds, LED_COUNT, CRGB::Green);
          FastLED.show();
          delay(75);
          fill_solid(leds, LED_COUNT, CRGB::Black);
          FastLED.show();
          delay(75);
        }

        EEPROM.write(4, countdown.getSeconds());
        EEPROM.write(5, countdown.getMinutes());
        EEPROM.write(6, countdown.getHours());
        EEPROM.commit();
        delay(250);
        
        setDisplay();
      }
    }
  }

  if (countdownStarted && countdown.isOver())
  {
    countdownStarted = false;
    showTimerEnd();
    setDisplay();
  }

  refreshDevice();
}


uint8_t lastSecond;
uint8_t hue;
void refreshDevice()
{
  if (countdownStarted)
  {
    if (lastSecond != countdown.getRemainingSeconds()) 
    {
      displayTimer(countdown, true);
      lastSecond = countdown.getRemainingSeconds();
    }
    if (isTimestamp(timestamp, 25))
    {
      if (!countdown.isPaused())
      {
        float percent = (float)countdown.getTotalRemainingMilliseconds() / countdown.getTotalMilliseconds();
        int count = percent * LED_COUNT;
        for (int i = 0; i < LED_COUNT; i++)
        {
          if (i < count)
            leds[i] = CRGB::Red;
          else if (i == count) 
          {
            float n = percent * LED_COUNT - count;
            int redValue = constrain((int)(n * 255), 0, 255);
            leds[i] = CRGB(redValue, 0, 0);
          }
          else
            leds[i] = CRGB::Black;
        }
        FastLED.show();
      }
      else 
      {
        fadeToBlackBy(leds, LED_COUNT, 16); // 16/255 = 6%
        EVERY_N_MILLISECONDS(250) {
          leds[random8(LED_COUNT)] = CRGB::Yellow;
        }
        FastLED.show();
      }
    }
  }
  else
  {
    if (isTimestamp(timestamp, 50))
    {
      hue++;
      fill_rainbow(leds, LED_COUNT, hue, 255 / LED_COUNT);
      FastLED.show();
    }
  }
}
void setDisplay()
{
  u8x8.clear();
  if (!countdownStarted)
  {
    u8x8.drawString(2, 0, "Focus Wheel!");
    displayTimer(countdown, false);
  }
}
void setEditMode(uint8_t h, uint8_t m, uint8_t s)
{
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

  u8x8.draw2x2String(0, 1, text.c_str());
}
void drawUnderline(uint8_t selection, uint8_t y)
{
  uint8_t tiles[8] = { 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0 };

  u8x8.clearLine(y);

  uint8_t start = (2 - selection) * 6;
  uint8_t end = start + 4;
  for (int i = start; i < end; i++)
    u8x8.drawTile(i, y, 1, tiles);
}
void displayTimer(Timer& t, bool started) 
{
  uint8_t s = started ? t.getRemainingSeconds() : t.getSeconds();
  uint8_t m = started ? t.getRemainingMinutes() : t.getMinutes();
  uint8_t h = started ? t.getRemainingHours() : t.getHours();

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


void editTimer()
{
  u8x8.clear();
  uint8_t selected = 0;

  int8_t t[3] = { countdown.getSeconds(), countdown.getMinutes(), countdown.getHours() };
  setEditMode(t[2], t[1], t[0]);
  drawUnderline(selected, 3);
  
  while (true) 
  {
    RotaryEncoder::Direction dir = encoder.getDirection();
    if (dir != RotaryEncoder::Direction::NOROTATION)
    {
      t[selected] += (dir == RotaryEncoder::Direction::CLOCKWISE) ? -1 : 1;

      if (t[selected] < 0)
        t[selected] = selected != 2 ? 59 : 3;
      else if (selected == 2 && t[selected] > 3 || selected != 2 && t[selected] > 59)
        t[selected] = 0;

      setEditMode(t[2], t[1], t[0]);
    }

    if (middleBtn())
    {
      playSound();
      while (middleBtn()) 
      {
        refreshDevice();
        delay(2);
      }
      selected = (selected + 1) % 3;
      setEditMode(t[2], t[1], t[0]);
      drawUnderline(selected, 3);
    }

    if (rightBtn())
    {
      playSound();
      while (rightBtn())
      {
        refreshDevice();
        delay(2);
      }
      break;
    }

    refreshDevice();
    delay(2);
  }

  countdown.setTimer(t[2], t[1], t[0]);
  setDisplay();
}
void showTimerEnd()
{
  u8x8.clear();
  u8x8.draw2x2String(2, 2, "ENDED!");

  fill_solid(leds, LED_COUNT, CRGB::Black);
  FastLED.show();

  unsigned long t = millis();
  uint8_t i = 0;
  uint8_t half = (LED_COUNT / 2);
  while (millis() - t < 10000)
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
      while (anyBtn()) { delay(2); }
      break;
    }
    
    delay(2);
  }
}