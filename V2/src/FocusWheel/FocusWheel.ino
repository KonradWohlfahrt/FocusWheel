#include "preferences.h"
#include "displayFunctions.h"


// https://github.com/KonradWohlfahrt/Arduino-Timer-Library
#include <DonutStudioTimer.h>
Timer workTimer = Timer();
Timer breakTimer = Timer();
Timer twentyMinutes = Timer(0, 20);
Timer eyeTimer = Timer(0, 0, 30);


struct FocusWheelTimers {
  int8_t focusHours;
  int8_t focusMinutes;
  int8_t focusSeconds;
  int8_t breakHours;
  int8_t breakMinutes;
  int8_t breakSeconds;
  bool activateEyeTimer;
};
FocusWheelTimers currentTimers[4];
uint8_t lastSecond = 60;


struct FocusWheelSettings {
  uint8_t ledBrightnessIndex;
  uint8_t displayContrast;
  bool soundEnabledDefault;
  uint8_t melodyIndex;
};
FocusWheelSettings currentSettings;
#define BRIGHTNESS_OPTIONS 5
uint8_t _brightnessOptions[] = { 10, 15, 20, 25, 30 };
bool isSoundEnabled = false;
bool isSelected = false;
uint8_t currentSettingsIndex = 0;


enum FocusWheelMode {
  SelectTimerMode,
  WorkPhaseMode,
  AwaitInputMode,
  AwaitEyeTimerMode,
  EyeTimerMode,
  BreakPhaseMode,
  SettingsMode
};
FocusWheelMode currentMode = SelectTimerMode;
uint8_t currentModeIndex = 0;


uint8_t hue = 0;
uint8_t ledIndex = 0;


IRAM_ATTR void checkPosition() { encoder.tick(); }
void setup() 
{
  // --- INITIALIZE DISPLAY ---
  u8x8.begin();
  u8x8.setFont(u8x8_font_pressstart2p_f);
  u8x8.draw2x2String(0, 0, "FOCUS");
  u8x8.draw2x2String(5, 2, "WHEEL");


  // --- INITIALIZE PINS ---
  pinMode(TOP_BTN_PIN, INPUT);
  pinMode(BOTTOM_BTN_PIN, INPUT);
  pinMode(ROTARY_ENCODER_BTN_PIN, INPUT);
  pinMode(CHARGE_PIN, INPUT);
  pinMode(BATTERY_READ_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  delay(250);


  // --- INITIALIZE EEPROM ---
  EEPROM.begin(1 + sizeof(FocusWheelSettings) + sizeof(FocusWheelTimers) * 4);
  delay(500);
  if (EEPROM.read(0) == 255) // if not used yet, EEPROM values will be 255
  {
    FocusWheelSettings defaultSettings(1, 50, true, 0);
    currentSettings = defaultSettings;

    FocusWheelTimers firstTimer(0, 30, 0, 0, 5, 0, false);
    FocusWheelTimers secondTimer(0, 45, 0, 0, 7, 0, true);
    FocusWheelTimers thirdTimer(1, 0, 0, 0, 15, 0, true);
    FocusWheelTimers fourthTimer(1, 30, 0, 0, 30, 0, true);
    currentTimers[0] = firstTimer;
    currentTimers[1] = secondTimer;
    currentTimers[2] = thirdTimer;
    currentTimers[3] = fourthTimer;

    EEPROM.write(0, 0);
    saveEEPROM();
  }
  else
    loadEEPROM();
  isSoundEnabled = currentSettings.soundEnabledDefault;
  u8x8.setContrast(currentSettings.displayContrast);
  u8x8.clearLine(0);
  

  // --- CHECK BATTERY ---
  checkBattery(true);
  

  // --- INITIALIZE LED RING ---
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, LED_COUNT).setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 350);
  FastLED.setBrightness(_brightnessOptions[currentSettings.ledBrightnessIndex]);


  // --- INITIALIZE TIMER SELECT MENU ---
  encoder.setPosition(0);
  drawTimerMenu(true);


  // --- ATTATCH INTERRUPTS FOR ROTARY ENCODER ---
  attachInterrupt(digitalPinToInterrupt(ROTARY_ENCODER_A_PIN), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ROTARY_ENCODER_B_PIN), checkPosition, CHANGE);
}
void loop()
{
  checkButtons();

  refreshTimers();
  updateLEDRing();
  checkBattery(false);

  if (currentMode == WorkPhaseMode)
  {
    if (currentTimers[currentModeIndex].activateEyeTimer && twentyMinutes.isOver())
    {
      // only start eye timer if ~10 minutes of the work timer remain
      if ((workTimer.getTotalRemainingMilliseconds() / 1000) >= 595)
        awaitEyeTimer();
    }
    if (workTimer.isOver())
      endCurrentTimer(false);
  }
  if (currentMode == BreakPhaseMode)
  {
    if (breakTimer.isOver())
      endCurrentTimer(false);
  }
  if (currentMode == EyeTimerMode)
  {
    if (eyeTimer.isOver())
      endCurrentTimer(false);
  }
}


void loadEEPROM() 
{
  uint8_t currentAddress = 1;
  EEPROM.get(currentAddress, currentSettings);
  currentAddress += sizeof(FocusWheelSettings);
  for (uint8_t i = 0; i < 4; i++)
  {
    EEPROM.get(currentAddress, currentTimers[i]);
    currentAddress += sizeof(FocusWheelTimers);
  }
}
void saveEEPROM()
{
  uint8_t currentAddress = 1;
  EEPROM.put(currentAddress, currentSettings);
  currentAddress += sizeof(FocusWheelSettings);
  for (uint8_t i = 0; i < 4; i++)
  {
    EEPROM.put(currentAddress, currentTimers[i]);
    currentAddress += sizeof(FocusWheelTimers);
  }
  delay(250);
  EEPROM.commit();
  delay(250);
}


void checkButtons()
{
  // enable/disable sound
  if (bottomBtn()) 
  {
    while (bottomBtn())
    {
      delay(1); // avoid stack overflow
      updateLEDRing();
      checkBattery(false);
    }

    isSoundEnabled = !isSoundEnabled;
    displaySoundSetting(isSoundEnabled);

    if (isSoundEnabled)
      beep();
  }

  // rotary encoder button
  if (middleBtn()) 
  {
    if (isSoundEnabled)
      beep();

    while (middleBtn())
    {
      delay(1); // avoid stack overflow
      updateLEDRing();
      checkBattery(false);
    }

    if (currentMode == SelectTimerMode)
    {
      currentModeIndex = encoder.getPosition() % 4;
      workTimer.setTimer(currentTimers[currentModeIndex].focusHours, currentTimers[currentModeIndex].focusMinutes, currentTimers[currentModeIndex].focusSeconds);
      breakTimer.setTimer(currentTimers[currentModeIndex].breakHours, currentTimers[currentModeIndex].breakMinutes, currentTimers[currentModeIndex].breakSeconds);

      currentMode = WorkPhaseMode;
      startCurrentTimer();
    }
    else if (currentMode == WorkPhaseMode) 
    {
      if (currentTimers[currentModeIndex].activateEyeTimer)
        twentyMinutes.setPause(!workTimer.isPaused());
      workTimer.setPause(!workTimer.isPaused());
    }
    else if (currentMode == AwaitInputMode)
    {
      currentMode = BreakPhaseMode;
      startCurrentTimer();
    }
    else if (currentMode == AwaitEyeTimerMode)
    {
      currentMode = EyeTimerMode;
      startCurrentTimer();
    }
    else if (currentMode == BreakPhaseMode)
      breakTimer.setPause(!breakTimer.isPaused());
    else if (currentMode == SettingsMode)
    {
      uint8_t pos = encoder.getPosition() % 8;
      if (!isSelected && pos >= 4)
      {
        currentSettingsIndex = pos;
        editTimer(pos - 4);

        encoder.setPosition(currentSettingsIndex);
        drawSettingsMenu(true);
        return;
      }

      isSelected = !isSelected;
      if (isSelected)
      {
        currentSettingsIndex = pos;
        if (currentSettingsIndex == 3)
          playMelody(melodys[currentSettings.melodyIndex], durations[currentSettings.melodyIndex]);
      } 
      else 
      {
        encoder.setPosition(currentSettingsIndex);
      }

      drawSettingsMenu(false);
    }
  }

  // skip or enter settings
  if (topBtn()) 
  {
    if (isSoundEnabled)
      beep();

    while (topBtn())
    {
      delay(1); // avoid stack overflow
      updateLEDRing();
      checkBattery(false);
    }

    if (currentMode == SelectTimerMode)
    {
      currentModeIndex = encoder.getPosition() % 4;
      currentMode = SettingsMode;
      currentSettingsIndex = 0;
      encoder.setPosition(0);
      isSelected = false;

      drawSettingsMenu(true);
    }
    else if (currentMode == WorkPhaseMode)
      endCurrentTimer(true);
    else if (currentMode == AwaitInputMode)
    {
      currentMode = BreakPhaseMode;
      startCurrentTimer();
    }
    else if (currentMode == AwaitEyeTimerMode)
    {
      currentMode = EyeTimerMode;
      endCurrentTimer(true);
    }
    else if (currentMode == EyeTimerMode)
      endCurrentTimer(true);
    else if (currentMode == BreakPhaseMode)
      endCurrentTimer(true);
    else if (currentMode == SettingsMode && !isSelected) 
    {
      saveEEPROM();

      encoder.setPosition(currentModeIndex);
      currentMode = SelectTimerMode;
      u8x8.clearLine(0);
      checkBattery(true);
      drawTimerMenu(true);
    }
  }

  // wheel rotated
  RotaryEncoder::Direction dir = encoder.getDirection();
  if (dir != RotaryEncoder::Direction::NOROTATION)
  {
    if (currentMode == WorkPhaseMode || currentMode == AwaitInputMode || currentMode == BreakPhaseMode)
      return;

    if(isSoundEnabled)
      beep(400, 20);

    if (encoder.getPosition() < 0)
    {
      if (currentMode == SelectTimerMode)
        encoder.setPosition(3);
      else if (currentMode == SettingsMode)
        encoder.setPosition(7);
    }

    if (currentMode == SelectTimerMode)
      drawTimerMenu(false);
    else if (currentMode == SettingsMode) 
    {
      if (isSelected)
      {
        if (dir == RotaryEncoder::Direction::CLOCKWISE)
        {
          switch(currentSettingsIndex)
          {
            case 0:
              currentSettings.ledBrightnessIndex = (currentSettings.ledBrightnessIndex + 1) % BRIGHTNESS_OPTIONS;
              FastLED.setBrightness(_brightnessOptions[currentSettings.ledBrightnessIndex]);
              break;
            case 1:
              if (currentSettings.displayContrast < 250)
                currentSettings.displayContrast += 50;
              else
                currentSettings.displayContrast = 0;
              u8x8.setContrast(currentSettings.displayContrast);
              break;
            case 2:
              currentSettings.soundEnabledDefault = !currentSettings.soundEnabledDefault;
              break;
            case 3:
              currentSettings.melodyIndex = (currentSettings.melodyIndex + 1) % MELODYS;
              playMelody(melodys[currentSettings.melodyIndex], durations[currentSettings.melodyIndex]);
              break;
          }
        }
        else if (dir == RotaryEncoder::Direction::COUNTERCLOCKWISE)
        {
          switch(currentSettingsIndex)
          {
            case 0:
              currentSettings.ledBrightnessIndex = (currentSettings.ledBrightnessIndex - 1) < 0 ? (BRIGHTNESS_OPTIONS - 1) : (currentSettings.ledBrightnessIndex - 1);
              FastLED.setBrightness(_brightnessOptions[currentSettings.ledBrightnessIndex]);
              break;
            case 1:
              if (currentSettings.displayContrast > 0)
                currentSettings.displayContrast -= 50;
              else
                currentSettings.displayContrast = 250;
              u8x8.setContrast(currentSettings.displayContrast);
              break;
            case 2:
              currentSettings.soundEnabledDefault = !currentSettings.soundEnabledDefault;
              break;
            case 3:
              currentSettings.melodyIndex = (currentSettings.melodyIndex - 1) < 0 ? (MELODYS - 1) : (currentSettings.melodyIndex - 1);
              playMelody(melodys[currentSettings.melodyIndex], durations[currentSettings.melodyIndex]);
              break;
          }
        }
      }
      drawSettingsMenu(false);
    }
  }
}
void checkBattery(bool force) 
{
  bool check = false;
  EVERY_N_SECONDS(5) { check = true; }

  if (check || force)
  {
    if (isBatteryPowered() && getBatteryLevel() <= 0)
    {
      beep(250);

      fill_solid(leds, LED_COUNT, 0);
      FastLED.show();

      u8x8.clear();
      u8x8.drawString(0, 0, "Low Battery");
      u8x8.drawTile(13, 0, 3, (uint8_t*)_batteryBackgroundCharging);
      u8x8.drawTile(14, 0, 1, (uint8_t*)_batteryLow);

      beep();

      esp_deep_sleep_start();
    }

    displaySoundSetting(isSoundEnabled);
    displayBatteryStatus(currentMode != SettingsMode);
  }
}



void updateLEDRing()
{
  if (currentMode == SelectTimerMode)
  {
    EVERY_N_MILLISECONDS(20)
    { 
      hue++;
      fill_rainbow(leds, LED_COUNT, hue, 255 / LED_COUNT);

      uint8_t start = (4 * (encoder.getPosition() % 4) + 5) % LED_COUNT;
      uint8_t end = start + 11;

      for (uint8_t i = start; i < end; i++)
        leds[i % LED_COUNT] = 0;

      FastLED.show();
    }
  }
  else if (currentMode == WorkPhaseMode)
  {
    drawCountdownRing(workTimer, CRGB::Red, CRGB::Yellow);
  }
  else if (currentMode == AwaitInputMode || currentMode == AwaitEyeTimerMode)
  {
    EVERY_N_MILLISECONDS(25) 
    {
      ledIndex = (ledIndex + 1) % LED_COUNT;
      leds[ledIndex] = CRGB::GhostWhite;
      fadeToBlackBy(leds, LED_COUNT, 64); // 64/255 = 25%
      FastLED.show(); 
    }
  }
  else if (currentMode == EyeTimerMode)
  {
    drawCountdownRing(eyeTimer, CRGB::Blue, CRGB::Yellow);
  }
  else if (currentMode == BreakPhaseMode)
  {
    drawCountdownRing(breakTimer, CRGB::Green, CRGB::Yellow);
  }
  else if (currentMode == SettingsMode)
  {
    EVERY_N_MILLISECONDS(20)
    { 
      hue++;
      fill_solid(leds, LED_COUNT, isSelected ? CRGB::Gold : CRGB::Orange);

      uint8_t index = isSelected ? currentSettingsIndex : encoder.getPosition() % 8;
      uint8_t start = (2 * index + 2) % LED_COUNT;
      uint8_t end = start + 14;

      for (uint8_t i = start; i < end; i++)
        leds[i % LED_COUNT] = 0;

      FastLED.show();
    }
  }
}
void refreshTimers()
{
  if (currentMode == WorkPhaseMode)
  {
    if (lastSecond != workTimer.getRemainingSeconds())
    {
      display2x2Countdown(workTimer, true, 1);
      lastSecond = workTimer.getRemainingSeconds();
    }
  }
  else if (currentMode == BreakPhaseMode)
  {
    if (lastSecond != breakTimer.getRemainingSeconds())
    {
      display2x2Countdown(breakTimer, true, 1);
      lastSecond = breakTimer.getRemainingSeconds();
    }
  }
  else if (currentMode == EyeTimerMode)
  {
    if (lastSecond != eyeTimer.getRemainingSeconds())
    {
      draw2x2Countdown(eyeTimer.getRemainingSeconds(), 1);
      lastSecond = eyeTimer.getRemainingSeconds();
    }
  }
}


void drawTimerMenu(bool fullReset)
{
  uint8_t index = encoder.getPosition() % 4;
  if (fullReset)
  {
    u8x8.clearLine(1);
    u8x8.clearLine(2);
    u8x8.drawString(0, 1, "Work:");
    u8x8.drawString(0, 2, "Break:");
  }
  
  drawCountdown(currentTimers[index].focusHours, currentTimers[index].focusMinutes, currentTimers[index].focusSeconds, 7, 1);
  drawCountdown(currentTimers[index].breakHours, currentTimers[index].breakMinutes, currentTimers[index].breakSeconds, 7, 2);
  drawSlider(3, 4, index);
}
void drawSettingsMenu(bool fullReset)
{
  uint8_t index = isSelected ? currentSettingsIndex : encoder.getPosition() % 8;

  if (fullReset)
  {
    checkBattery(true);
    u8x8.drawString(2, 0, "Settings");
  }

  u8x8.clearLine(1);
  u8x8.clearLine(2);
  if (index == 0)
  {
    u8x8.drawString(0, 1, "Brightness");
    if (isSelected)
      u8x8.setInverseFont(true);
    u8x8.draw2x2String(11, 1, String(_brightnessOptions[currentSettings.ledBrightnessIndex]).c_str());
    if (isSelected)
      u8x8.setInverseFont(false);
  }
  else if (index == 1) 
  {
    u8x8.drawString(0, 1, "Contrast");
    if (isSelected)
      u8x8.setInverseFont(true);
    if (currentSettings.displayContrast >= 100)
      u8x8.draw2x2String(9, 1, String(currentSettings.displayContrast).c_str());
    else if (currentSettings.displayContrast >= 10)
      u8x8.draw2x2String(11, 1, String(currentSettings.displayContrast).c_str());
    else
      u8x8.draw2x2String(13, 1, String(currentSettings.displayContrast).c_str());
    if (isSelected)
      u8x8.setInverseFont(false);
  }
  else if (index == 2) 
  {
    u8x8.drawString(0, 1, "Default");
    u8x8.drawString(0, 2, "Sound");
    if (isSelected)
      u8x8.setInverseFont(true);
    if (currentSettings.soundEnabledDefault)
      u8x8.draw2x2String(11, 1, "ON");
    else
      u8x8.draw2x2String(9, 1, "OFF");
    if (isSelected)
      u8x8.setInverseFont(false);
  }
  else if (index == 3) 
  {
    u8x8.drawString(0, 1, "Melody");
    if (isSelected)
      u8x8.setInverseFont(true);
    u8x8.draw2x2String(14, 1, String(currentSettings.melodyIndex + 1).c_str());
    if (isSelected)
      u8x8.setInverseFont(false);
  }
  else
  {
    u8x8.draw2x2String(0, 1, ("Timer " + String(index - 3)).c_str());
  }

  drawSlider(3, 8, index);
}


void display2x2Countdown(Timer &t, bool started, uint8_t y) 
{
  uint8_t s = started ? t.getRemainingSeconds() : t.getSeconds();
  uint8_t m = started ? t.getRemainingMinutes() : t.getMinutes();
  uint8_t h = started ? t.getRemainingHours() : t.getHours();

  draw2x2Countdown(h, m, s, y);
}
void drawCountdownRing(Timer &t, CRGB color, CRGB pauseColor)
{
  if (!t.isPaused()) 
  {
    EVERY_N_MILLISECONDS(25) 
    {
      double percent = (double)t.getTotalRemainingMilliseconds() / t.getTotalMilliseconds();
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
      fadeToBlackBy(leds, LED_COUNT, 16); // 16/255 ~ 6%
      FastLED.show();
    }
    EVERY_N_MILLISECONDS(250) 
    {
      leds[random8(LED_COUNT)] = pauseColor;
      FastLED.show();
    }
  }
}


void startCurrentTimer()
{
  lastSecond = 60;
  if (currentMode == WorkPhaseMode)
  {
    workTimer.begin();
    if (currentTimers[currentModeIndex].activateEyeTimer)
      twentyMinutes.begin();
      
    u8x8.clear();
    checkBattery(true);
    drawCountdownText(3, "Work Phase!");
  }
  else if (currentMode == AwaitInputMode)
  {
    checkBattery(true);
    display2x2Countdown(breakTimer, false, 1);
    drawCountdownText(2, "Break Phase!");
  }
  else if (currentMode == EyeTimerMode)
  {
    eyeTimer.begin();
    checkBattery(true);
    draw2x2Countdown(eyeTimer.getSeconds(), 1);
    drawCountdownText(3, "20-20-Rule");
  }
  else if (currentMode == BreakPhaseMode)
  {
    breakTimer.begin();
  }
}
void endCurrentTimer(bool skipFiller)
{
  if (currentMode == WorkPhaseMode)
  {
    if (!skipFiller)
    {
      timerEndAnimation(CRGB::Red);
      currentMode = AwaitInputMode;
    }
    else 
    {
      currentMode = BreakPhaseMode;
      checkBattery(true);
      drawCountdownText(2, "Break Phase!");
    }

    startCurrentTimer();
  }
  else if (currentMode == BreakPhaseMode)
  {
    if (!skipFiller)
      timerEndAnimation(CRGB::Green);
    currentMode = SelectTimerMode;

    u8x8.clear();
    encoder.setPosition(currentModeIndex);
    drawTimerMenu(true);
    checkBattery(true);
  }
  else if (currentMode == EyeTimerMode)
  {
    if (!skipFiller && isSoundEnabled)
      doubleBeep();

    currentMode = WorkPhaseMode;
    workTimer.setPause(false);
    twentyMinutes.begin();

    lastSecond = 60;
    drawCountdownText(3, "Work Phase!");
    checkBattery(true);
  }
}
void awaitEyeTimer()
{
  if (isSoundEnabled)
    doubleBeep();

  lastSecond = 60;
  workTimer.setPause(true);
  currentMode = AwaitEyeTimerMode;
  u8x8.clear();
  checkBattery(true);
  draw2x2Countdown(eyeTimer.getSeconds(), 1);
  drawCountdownText(3, "20-20-Rule");
}
void timerEndAnimation(CRGB color)
{
  u8x8.clear();
  u8x8.draw2x2String(4, 2, "END!");

  uint8_t n = 0;
  uint8_t half = (LED_COUNT / 2);

  uint32_t time = millis();
  uint32_t next = 0;
  uint8_t i = 0;

  while (true)
  {
    EVERY_N_MILLISECONDS(25)
    {
      n = (n + 1) % half;
      leds[n] = color;
      leds[n + half] = color;
      fadeToBlackBy(leds, LED_COUNT, 96); // 96/255 = ~38%
      FastLED.show(); 
    }

    if (millis() - time >= next)
    {
      uint16_t dur = noteDuration(durations[currentSettings.melodyIndex][i]);
      if (isSoundEnabled)
        beep(melodys[currentSettings.melodyIndex][i], dur);
      i++;
      next = dur * 1.3;
      time = millis();
    }

    if (i >= MELODY_LENGTH)
        break;
    
    delay(1);
  }
}


void editTimer(uint8_t timerIndex)
{
  u8x8.clear();
  uint8_t selected = 0;

  draw2x2Countdown(currentTimers[timerIndex].focusHours, currentTimers[timerIndex].focusMinutes, currentTimers[timerIndex].focusSeconds, 1);
  drawUnderline(selected, 3);
  u8x8.drawString(6, 0, "Work");
  
  int8_t x = 0;
  while (true) 
  {
    if (getDirection(x)) 
    {
      if (isSoundEnabled)
        beep(400, 20);

      if (selected == 6)
      {
        currentTimers[timerIndex].activateEyeTimer = !currentTimers[timerIndex].activateEyeTimer;
      }
      else
      {
        switch (selected)
        {
          case 0:
            currentTimers[timerIndex].focusSeconds = changeTimerValue(selected, currentTimers[timerIndex].focusSeconds, x);
            break;
          case 1:
            currentTimers[timerIndex].focusMinutes = changeTimerValue(selected, currentTimers[timerIndex].focusMinutes, x);
            break;
          case 2:
            currentTimers[timerIndex].focusHours = changeTimerValue(selected, currentTimers[timerIndex].focusHours, x);
            break;
          case 3:
            currentTimers[timerIndex].breakSeconds = changeTimerValue(selected, currentTimers[timerIndex].breakSeconds, x);
            break;
          case 4:
            currentTimers[timerIndex].breakMinutes = changeTimerValue(selected, currentTimers[timerIndex].breakMinutes, x);
            break;
          case 5:
            currentTimers[timerIndex].breakHours = changeTimerValue(selected, currentTimers[timerIndex].breakHours, x);
            break;
        }
      
      }
      x = 0;

      if (selected == 6)
      {
        if (currentTimers[timerIndex].activateEyeTimer)
          u8x8.draw2x2String(9, 1, " ");
        u8x8.setInverseFont(true);
        if (currentTimers[timerIndex].activateEyeTimer)
        {
          u8x8.draw2x2String(11, 1, "ON");
        }
        else
          u8x8.draw2x2String(9, 1, "OFF");
        u8x8.setInverseFont(false);
      }
      else if (selected >= 3)
        draw2x2Countdown(currentTimers[timerIndex].breakHours, currentTimers[timerIndex].breakMinutes, currentTimers[timerIndex].breakSeconds, 1);
      else
        draw2x2Countdown(currentTimers[timerIndex].focusHours, currentTimers[timerIndex].focusMinutes, currentTimers[timerIndex].focusSeconds, 1);
    }

    // change
    if (bottomBtn() || topBtn())
    {
      if (isSoundEnabled)
        beep();

      if (topBtn())
        selected = (selected - 1) < 0 ? 6 : (selected - 1);
      else
        selected = (selected + 1) % 7;

      while (bottomBtn() || topBtn())
        delay(1);
      
      if (selected != 6)
      {
        drawUnderline(selected % 3, 3);
        if (selected == 3 || selected == 5) 
        {
          u8x8.clearLine(0);
          u8x8.drawString(5, 0, "Break");
          draw2x2Countdown(currentTimers[timerIndex].breakHours, currentTimers[timerIndex].breakMinutes, currentTimers[timerIndex].breakSeconds, 1);
        }
        else if (selected == 0 || selected == 2)
        {
          u8x8.clearLine(0);
          u8x8.drawString(6, 0, "Work");
          draw2x2Countdown(currentTimers[timerIndex].focusHours, currentTimers[timerIndex].focusMinutes, currentTimers[timerIndex].focusSeconds, 1);
        }
      }
      else
      {
        u8x8.clear();
        u8x8.drawString(3, 0, "20-20-Rule");
        u8x8.drawString(0, 1, "Eye");
        u8x8.drawString(0, 2, "Timer");

        u8x8.setInverseFont(true);
        if (currentTimers[timerIndex].activateEyeTimer)
          u8x8.draw2x2String(11, 1, "ON");
        else
          u8x8.draw2x2String(9, 1, "OFF");
        u8x8.setInverseFont(false);
      }
    }

    // finish
    if (middleBtn())
    {
      if (isSoundEnabled)
        beep();
      while (middleBtn())
        delay(1);
      break;
    }

    delay(1);
  }

  u8x8.clear();
}
int8_t changeTimerValue(uint8_t selected, int8_t current, int8_t value)
{
  current += value;
  if (current < 0)
    current = (selected != 2 && selected != 5) ? 59 : 3;
  else if ((selected == 2 || selected == 5) && current > 3 || (selected != 2 || selected != 5) && current > 59)
    current = 0;
  return current;
}
bool getDirection(int8_t& value)
{
  RotaryEncoder::Direction dir = encoder.getDirection();
  if (dir != RotaryEncoder::Direction::NOROTATION)
  {
    value += (dir == RotaryEncoder::Direction::CLOCKWISE) ? 1 : -1;
    return true;
  }
  return false;
}