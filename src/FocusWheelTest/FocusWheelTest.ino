#include "preferences.h"

uint8_t hue = 0;
long lastPos = 0;
unsigned long tmsp;

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
  u8x8.draw1x2String(0, 1, "Test!");

  // --- INITIALIZE EEPROM AND LOAD SLIDER VALUES ---
  EEPROM.begin(6);
  delay(250);
  loadSliderValues();


  // -------------
  // BEGIN TESTING
  // -------------


  // --- TEST BUZZER ---
  for (int i = 0; i < 3; i++)
  {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }

  // --- TEST LED RING ---
  for (int i = 0; i < LED_COUNT; i++)
  {
    leds[i] = CRGB(255, 0, 0);
    FastLED.show();
    delay(75);
  }
  for (int i = 0; i < LED_COUNT; i++)
  {
    leds[i] = CRGB(0, 0, 0);
    FastLED.show();
    delay(75);
  }

  u8x8.clear();
  u8x8.drawString(0, 0, "Slider: ");
  u8x8.drawString(0, 1, "Position: 0");
}
void loop()
{
  EVERY_N_MILLISECONDS(20) { hue++; }
  fill_rainbow(leds, LED_COUNT, hue, 255 / LED_COUNT);
  FastLED.show();

  if (millis() - tmsp > 25)
  {
    uint8_t s = getSliderPosition();
    
    if (s < 10)
      u8x8.drawString(8, 0, ("00" + String(s)).c_str());
    else if (s < 100)
      u8x8.drawString(8, 0, ("0" + String(s)).c_str());
    else
      u8x8.drawString(8, 0, String(s).c_str());
    
    u8x8.drawString(0, 3, String(middleBtn()).c_str());
    u8x8.drawString(1, 3, String(rightBtn()).c_str());
    tmsp = millis();
  }

  long newPos = encoder.getPosition();
  if (lastPos != newPos)
  {
    u8x8.clearLine(2);
    u8x8.drawString(0, 1, ("Position: " + String(newPos)).c_str());
    lastPos = newPos;
  }
}