#include "preferences.h"

uint8_t hue = 0;
IRAM_ATTR void checkPosition() { encoder.tick(); }

void setup() 
{
  Serial.begin(115200);

  // --- INITIALIZE PINS ---
  pinMode(TOP_BTN_PIN, INPUT);
  pinMode(BOTTOM_BTN_PIN, INPUT);
  pinMode(ROTARY_ENCODER_BTN_PIN, INPUT);
  pinMode(CHARGE_PIN, INPUT);
  pinMode(BATTERY_READ_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // --- ATTATCH INTERRUPTS FOR ROTARY ENCODER ---
  attachInterrupt(digitalPinToInterrupt(ROTARY_ENCODER_A_PIN), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ROTARY_ENCODER_B_PIN), checkPosition, CHANGE);

  // --- INITIALIZE DISPLAY ---
  u8x8.begin();
  u8x8.setFont(u8x8_font_pressstart2p_f);
  u8x8.draw1x2String(0, 1, "Test!");

  // --- INITIALIZE LED RING ---
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, LED_COUNT).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(25);

  // --- TEST BUZZER ---
  for (int i = 0; i < 3; i++)
  {
    tone(BUZZER_PIN, 1000, 250);
    delay(375);
    beep();
    delay(175);
  }

  // --- TEST LED RING ---
  for (int i = 0; i < LED_COUNT; i++)
  {
    leds[i] = CRGB(255, 0, 0);
    FastLED.show();
    delay(50);
  }
  for (int i = 0; i < LED_COUNT; i++)
  {
    leds[i] = CRGB(0, 0, 0);
    FastLED.show();
    delay(50);
  }

  u8x8.clear();
}
void loop()
{
  EVERY_N_MILLISECONDS(20) { 
    hue++;
    fill_rainbow(leds, LED_COUNT, hue, 255 / LED_COUNT);
    FastLED.show();
  }

  EVERY_N_MILLISECONDS(100) {
    Serial.print(bottomBtn()); // bottom button pressed?
    Serial.print('\t');
    Serial.print(middleBtn()); // middle button pressed?
    Serial.print('\t');
    Serial.print(topBtn()); // top button pressed?
    Serial.print('\t');
    Serial.print(isBatteryPowered()); // is battery powered?
    Serial.print('\t');
    Serial.print(encoder.getPosition()); // encoder position
    Serial.print('\t');
    Serial.print(readBatteryVoltage()); // battery voltage in V
    Serial.print('\t');
    Serial.println(getBatteryLevel()); // battery level in %
    //Serial.print('\t');
    //Serial.println(isCharging()); // is charging?

    if (middleBtn()) {
      Serial.println(isCharging());
      u8x8.drawGlyph(14, 3, (isCharging() ? 1 : 0) + 48);
    }
  }

  EVERY_N_SECONDS(1) {
    u8x8.clear();
    u8x8.drawGlyph(0, 0, topBtn() + 48);
    u8x8.drawGlyph(2, 0, middleBtn() + 48);
    u8x8.drawGlyph(4, 0, bottomBtn() + 48);
    u8x8.drawGlyph(6, 0, isBatteryPowered() + 48);
    u8x8.drawString(8, 0, String(encoder.getPosition()).c_str());
    u8x8.drawString(10, 1, (String(readBatteryVoltage(), 2) + "V").c_str());
    u8x8.drawString(0, 1, (String(getBatteryLevel()) + "%").c_str());
  }
}