#include "preferences.h"

#include <DonutStudioTimer.h>
Timer workTimer = Timer();
Timer breakTimer = Timer();

IRAM_ATTR void checkPosition() { encoder.tick(); }


const CRGB _timerColors[5] = { CHSV(25, 255, 255), CHSV(76, 255, 255), CHSV(127, 255, 255), CHSV(178, 255, 255), CHSV(229, 255, 255) };


uint8_t _lineTiles[8] = { 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0 };
uint8_t _workTimes[5][3] = {
//  h,  m, s
  { 0, 30, 0 },
  { 0, 45, 0 },
  { 1, 0, 0 },
  { 1, 30, 0 },
  { 0, 0, 0 } // can be edited, saved to eeprom
};
uint8_t _breakTimes[5][3] = {
//  h, m, s
  { 0, 5, 0 },
  { 0, 7, 0 },
  { 0, 15, 0 },
  { 0, 30, 0 },
  { 0, 0, 0 } // can be edited, saved to eeprom
};

int8_t selectedTimer = 0;
bool timerStarted = false;
bool isInWorkPhase = true;

uint8_t ledIndex;


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
  EEPROM.begin(10);
  delay(250);
  loadSliderValues();
  _workTimes[4][0] = constrain(EEPROM.read(4), 0, 10); // hours
  _workTimes[4][1] = constrain(EEPROM.read(5), 0, 59); // minutes
  _workTimes[4][2] = constrain(EEPROM.read(6), 0, 59); // seconds
  _breakTimes[4][0] = constrain(EEPROM.read(7), 0, 10); // hours
  _breakTimes[4][1] = constrain(EEPROM.read(8), 0, 59); // minutes
  _breakTimes[4][2] = constrain(EEPROM.read(9), 0, 59); // seconds

  workTimer.setTimer(_workTimes[0][0], _workTimes[0][1], _workTimes[0][2]);
  breakTimer.setTimer(_breakTimes[0][0], _breakTimes[0][1], _breakTimes[0][2]);
  drawSelectTimer(true);
}
void loop()
{
  if (middleBtn())
  {
    playSound();
    while(middleBtn()) 
    {
      refreshLEDs();
      delay(2);
    }

    // start timer
    if (!timerStarted)
    {
      timerStarted = true;
      isInWorkPhase = true;
      workTimer.setTimer(_workTimes[selectedTimer][0], _workTimes[selectedTimer][1], _workTimes[selectedTimer][2]);
      breakTimer.setTimer(_breakTimes[selectedTimer][0], _breakTimes[selectedTimer][1], _breakTimes[selectedTimer][2]);
      workTimer.begin();
      u8x8.clear();
      u8x8.drawString(3, 0, "Work Phase!");
    }
    // pause timer
    else
    {
      if (isInWorkPhase)
        workTimer.setPause(!workTimer.isPaused());
      else
        breakTimer.setPause(!breakTimer.isPaused());
    }
  }

  if (rightBtn())
  {
    playSound();
    while(rightBtn()) 
    {
      refreshLEDs();
      delay(2);
    }

    if (!timerStarted)
      editTimer();
    else
      endCurrentTimer(false);
  }
  
  if (!timerStarted) 
  {
    if (getDirection(selectedTimer)) 
    {
      if (selectedTimer < 0)
        selectedTimer = 4;
      else if (selectedTimer > 4)
        selectedTimer = 0;
      
      drawSelectTimer(false);
    }
  }
  else
  {
    if (isInWorkPhase)
    {
      if (workTimer.isOver()) 
        endCurrentTimer(true);
    }
    else
    {
      if (breakTimer.isOver())
        endCurrentTimer(true);
    }
  }

  refreshLEDs();
  refreshDisplay();
}

void drawCountdownRing(Timer& t, CRGB color)
{
  if (!t.isPaused()) 
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
  else 
  {
    EVERY_N_MILLISECONDS(25) 
    {
      fadeToBlackBy(leds, LED_COUNT, 16); // 16/255 = 6%
      FastLED.show();
    }
    EVERY_N_MILLISECONDS(250) 
    {
      leds[random8(LED_COUNT)] = CRGB::Yellow;
      FastLED.show();
    }
  }
}
void drawCountdown(Timer& t, bool started, uint8_t y) 
{
  uint8_t s = started ? t.getRemainingSeconds() : t.getSeconds();
  uint8_t m = started ? t.getRemainingMinutes() : t.getMinutes();
  uint8_t h = started ? t.getRemainingHours() : t.getHours();

  drawCountdown(h, m, s, y);
}
void drawCountdown(uint8_t h, uint8_t m, uint8_t s, uint8_t y) 
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

  u8x8.draw2x2String(0, y, text.c_str());
}
void drawUnderline(uint8_t selection, uint8_t y)
{
  u8x8.clearLine(y);

  uint8_t start = (2 - selection) * 6;
  uint8_t end = start + 4;
  for (int i = start; i < end; i++)
    u8x8.drawTile(i, y, 1, _lineTiles);
}
void drawSelectTimer(bool fullReset)
{
  if (fullReset) 
  {
    u8x8.drawString(2, 0, "Focus Wheel!");
    u8x8.drawString(0, 1, "Work:");
    u8x8.drawString(0, 2, "Break:");
  }
  
  // WORK TIME
  String s = "";
  if (_workTimes[selectedTimer][0] < 10)
    s += "0";
  s += String(_workTimes[selectedTimer][0]) + ":";
  if (_workTimes[selectedTimer][1] < 10)
    s += "0";
  s += String(_workTimes[selectedTimer][1]) + ":";
  if (_workTimes[selectedTimer][2] < 10)
    s += "0";
  s += String(_workTimes[selectedTimer][2]);
  u8x8.drawString(7, 1, s.c_str());

  // BREAK TIME
  s = "";
  if (_breakTimes[selectedTimer][0] < 10)
    s += "0";
  s += String(_breakTimes[selectedTimer][0]) + ":";
  if (_breakTimes[selectedTimer][1] < 10)
    s += "0";
  s += String(_breakTimes[selectedTimer][1]) + ":";
  if (_breakTimes[selectedTimer][2] < 10)
    s += "0";
  s += String(_breakTimes[selectedTimer][2]);
  u8x8.drawString(7, 2, s.c_str());

  // select indication
  u8x8.clearLine(3);
  uint8_t start = selectedTimer * 3;
  uint8_t end = start + 3;
  for (int i = start; i < end; i++)
    u8x8.drawTile(i, 3, 1, _lineTiles);
}
bool getDirection(int8_t& value)
{
  RotaryEncoder::Direction dir = encoder.getDirection();
  if (dir != RotaryEncoder::Direction::NOROTATION)
  {
    value += (dir == RotaryEncoder::Direction::CLOCKWISE) ? -1 : 1;
    return true;
  }
  return false;
}


uint8_t lastSecond;
void refreshDisplay()
{
  if (timerStarted) 
  {
    if (isInWorkPhase)
    {
      if (lastSecond != workTimer.getRemainingSeconds()) 
      {
        drawCountdown(workTimer, true, 2);
        lastSecond = workTimer.getRemainingSeconds();
      }
    }
    else
    {
      if (lastSecond != breakTimer.getRemainingSeconds()) 
      {
        drawCountdown(breakTimer, true, 2);
        lastSecond = breakTimer.getRemainingSeconds();
      }
    }
  }
}
void refreshLEDs()
{
  if (!timerStarted) 
  {
    EVERY_N_MILLISECONDS(50)
    {
      ledIndex = (ledIndex + 1) % LED_COUNT;
      leds[ledIndex] = _timerColors[selectedTimer];
      fadeToBlackBy(leds, LED_COUNT, 64); // 64/255 = 25%
      FastLED.show();
    }
  }
  else
  {
    if (isInWorkPhase)
      drawCountdownRing(workTimer, CRGB::Red);
    else
      drawCountdownRing(breakTimer, CRGB::Green);
  }
}


void endCurrentTimer(bool alarm)
{
  if (isInWorkPhase)
  {
    if (alarm)
      showTimerEnd(CRGB::Red);
    isInWorkPhase = false;

    u8x8.clear();
    u8x8.drawString(2, 0, "Break Phase!");
    drawCountdown(breakTimer, false, 2);

    while (alarm)
    {
      if (anyBtn())
      {
        playSound();
        break;
      }

      EVERY_N_MILLISECONDS(25) 
      {
        ledIndex = (ledIndex + 1) % LED_COUNT;
        leds[ledIndex] = CRGB::GhostWhite;
        fadeToBlackBy(leds, LED_COUNT, 64); // 64/255 = 25%
        FastLED.show(); 
      }

      delay(2);
    }

    breakTimer.begin();
  }
  else
  {
    if (alarm)
      showTimerEnd(CRGB::Green);
    u8x8.clear();
    drawSelectTimer(true);
    timerStarted = false;
  }
}
void showTimerEnd(CRGB color)
{
  u8x8.clear();
  u8x8.draw2x2String(4, 2, "END!");

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
      leds[i] = color;
      leds[i + half] = color;
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


void editTimer()
{
  u8x8.clear();
  uint8_t selected = 0;

  int8_t t[6] = { _workTimes[selectedTimer][2], _workTimes[selectedTimer][1], _workTimes[selectedTimer][0], _breakTimes[selectedTimer][2], _breakTimes[selectedTimer][1], _breakTimes[selectedTimer][0] };
  drawCountdown(t[2], t[1], t[0], 1);
  drawUnderline(selected, 3);
  u8x8.drawString(6, 0, "Work");
  
  int8_t x = 0;
  while (true) 
  {
    if (getDirection(x)) 
    {
      t[selected] += x; x = 0;
      if (t[selected] < 0)
        t[selected] = (selected != 2 && selected != 5) ? 59 : 10;
      else if ((selected == 2 || selected == 5) && t[selected] > 10 || (selected != 2 || selected != 5) && t[selected] > 59)
        t[selected] = 0;

      if (selected >= 3)
        drawCountdown(t[5], t[4], t[3], 1);
      else
        drawCountdown(t[2], t[1], t[0], 1);
    }

    if (middleBtn())
    {
      playSound();
      while (middleBtn()) 
      {
        refreshLEDs();
        delay(2);
      }
      selected = (selected + 1) % 6;
      drawUnderline(selected % 3, 3);
      if (selected == 3) 
      {
        u8x8.clearLine(0);
        u8x8.drawString(5, 0, "Break");
        drawCountdown(t[5], t[4], t[3], 1);
      }
      else if (selected == 0)
      {
        u8x8.clearLine(0);
        u8x8.drawString(6, 0, "Work");
        drawCountdown(t[2], t[1], t[0], 1);
      }
    }

    if (rightBtn())
    {
      playSound();
      while (rightBtn())
      {
        refreshLEDs();
        delay(2);
      }
      break;
    }

    refreshLEDs();
    delay(2);
  }

  _workTimes[selectedTimer][2] = t[0];
  _workTimes[selectedTimer][1] = t[1];
  _workTimes[selectedTimer][0] = t[2];
  _breakTimes[selectedTimer][2] = t[3];
  _breakTimes[selectedTimer][1] = t[4];
  _breakTimes[selectedTimer][0] = t[5];

  if (selectedTimer == 4)
  {
    EEPROM.write(4, _workTimes[4][0]);
    EEPROM.write(5, _workTimes[4][1]);
    EEPROM.write(6, _workTimes[4][2]);
    EEPROM.write(7, _breakTimes[4][0]);
    EEPROM.write(8, _breakTimes[4][1]);
    EEPROM.write(9, _breakTimes[4][2]);
    EEPROM.commit();
    delay(250);
  }

  u8x8.clear();
  drawSelectTimer(true);
}