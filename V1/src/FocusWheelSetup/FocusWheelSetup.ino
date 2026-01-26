#include "preferences.h"


void setup() 
{
  pinMode(SLIDER_PIN, INPUT);
  analogSetAttenuation(ADC_2_5db);

  pinMode(SIDE_BTN_PIN, INPUT);
  pinMode(ROTARY_ENC_BTN_PIN, INPUT);

  u8x8.begin();
  u8x8.setFont(u8x8_font_pressstart2p_f);


  EEPROM.begin(6);
  delay(250);
  loadSliderValues();

  u8x8.drawString(0, 2, ("Min: " + String(_sliderMin)).c_str());
  u8x8.drawString(0, 3, ("Max: " + String(_sliderMax)).c_str());

  delay(2000);

  u8x8.clear();
  u8x8.draw1x2String(0, 1, "Slider down");
  delay(3000);
  _sliderMin = readSlider();

  u8x8.clear();
  u8x8.draw1x2String(0, 1, "Slider up");
  delay(3000);
  _sliderMax = readSlider();

  u8x8.clear();
  u8x8.drawString(0, 0, "Save?");
  u8x8.drawString(0, 1, "Press any button");
  u8x8.drawString(0, 2, ("Min: " + String(_sliderMin)).c_str());
  u8x8.drawString(0, 3, ("Max: " + String(_sliderMax)).c_str());

  delay(3000);

  if (anyBtn())
  {
    EEPROM.writeUInt(0, _sliderMin);
    EEPROM.writeUInt(2, _sliderMax);
    EEPROM.commit();
    u8x8.clear();
    u8x8.draw1x2String(0, 1, "Saved!");
    delay(2000);
  }

  u8x8.clear();
  u8x8.draw1x2String(0, 0, "Setup Done!");
  u8x8.drawString(0, 2, "Press any button");
  u8x8.drawString(0, 3, "to restart");
}

void loop()
{
  if (anyBtn())
    ESP.restart();
}
