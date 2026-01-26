#include "tiles.h"


/*
  --- TILE AND SLIDER ---
*/
void drawTilesFor(uint8_t y, uint8_t start, uint8_t end, uint8_t *tile)
{
  start = constrain(start, 0, 16);
  end = constrain(end + 1, 0, 16);
  for (int i = start; i < end; i++)
    u8x8.drawTile(i, y, 1, tile);
}
void drawSlider(uint8_t y, uint8_t amountOptions, uint8_t selection)
{
  uint8_t div = 16 / amountOptions;

  drawTilesFor(y, 0, 15, (uint8_t*)_bar);
  uint8_t start = selection * div;
  uint8_t end = start + div;
  drawTilesFor(y, start + 1, end - 2, (uint8_t*)_barFull);
  u8x8.drawTile(start, y, 1, (uint8_t*)_barLeft);
  u8x8.drawTile(end - 1, y, 1, (uint8_t*)_barRight);
}
void drawUnderline(uint8_t selection, uint8_t y)
{
  u8x8.clearLine(y);

  uint8_t start = (2 - selection) * 6;
  uint8_t end = start + 4;
  for (int i = start; i < end; i++)
    u8x8.drawTile(i, y, 1, (uint8_t*)_bar);
}


/*
  --- COUNTDOWN ---
*/
void draw2x2Countdown(uint8_t h, uint8_t m, uint8_t s, uint8_t y)
{
  u8x8.draw2x2Glyph(0, y, (h / 10 % 10) + 48);
  u8x8.draw2x2Glyph(2, y, (h % 10) + 48);

  u8x8.draw2x2Glyph(4, y, ':');

  u8x8.draw2x2Glyph(6, y, (m / 10 % 10) + 48);
  u8x8.draw2x2Glyph(8, y, (m % 10) + 48);

  u8x8.draw2x2Glyph(10, y, ':');

  u8x8.draw2x2Glyph(12, y, (s / 10 % 10) + 48);
  u8x8.draw2x2Glyph(14, y, (s % 10) + 48);
}
void draw2x2Countdown(uint8_t h, uint8_t m, uint8_t y)
{
  u8x8.draw2x2Glyph(3, y, (h / 10 % 10) + 48);
  u8x8.draw2x2Glyph(5, y, (h % 10) + 48);

  u8x8.draw2x2Glyph(7, y, ':');

  u8x8.draw2x2Glyph(9, y, (m / 10 % 10) + 48);
  u8x8.draw2x2Glyph(11, y, (m % 10) + 48);
}
void draw2x2Countdown(uint8_t t, uint8_t y)
{
  u8x8.draw2x2Glyph(6, y, (t / 10 % 10) + 48);
  u8x8.draw2x2Glyph(8, y, (t % 10) + 48);
}
void drawCountdown(uint8_t h, uint8_t m, uint8_t s, uint8_t x, uint8_t y)
{
  x = constrain(x, 0, 7);
  u8x8.drawGlyph(x, y, (h / 10 % 10) + 48);
  u8x8.drawGlyph(x + 1, y, (h % 10) + 48);

  u8x8.drawGlyph(x + 2, y, ':');

  u8x8.drawGlyph(x + 3, y, (m / 10 % 10) + 48);
  u8x8.drawGlyph(x + 4, y, (m % 10) + 48);

  u8x8.drawGlyph(x + 5, y, ':');

  u8x8.drawGlyph(x + 6, y, (s / 10 % 10) + 48);
  u8x8.drawGlyph(x + 7, y, (s % 10) + 48);
}
void drawCountdown(uint8_t h, uint8_t m, uint8_t x, uint8_t y)
{
  x = constrain(x, 0, 10);
  u8x8.drawGlyph(x, y, (h / 10 % 10) + 48);
  u8x8.drawGlyph(x + 1, y, (h % 10) + 48);

  u8x8.drawGlyph(x + 2, y, ':');

  u8x8.drawGlyph(x + 3, y, (m / 10 % 10) + 48);
  u8x8.drawGlyph(x + 4, y, (m % 10) + 48);
}
void drawCountdownText(uint8_t x, const char *s)
{
  u8x8.drawString(x, 3, s);
}


/*
    --- SOUND TILE ---
*/
void displaySoundSetting(bool enabled)
{
  u8x8.drawTile(0, 0, 1, enabled ? (uint8_t*)_soundOnTile : (uint8_t*)_soundOffTile);
}


/*
  --- BATTERY DISPLAY ---
*/
void displayBatteryStatus(bool showPercent)
{
  if (!isCharging()) 
  {
    uint8_t batLevel = getBatteryLevel();

    u8x8.drawTile(13, 0, 3, (uint8_t*)_batteryBackgroundNormal);
    if (batLevel >= 25)
      u8x8.drawTile(13, 0, 1, (uint8_t*)_batteryOneBar);
    if (batLevel >= 75)
      u8x8.drawTile(14, 0, 2, (uint8_t*)_batteryThreeBar);
    else if (batLevel >= 50)
      u8x8.drawTile(14, 0, 1, (uint8_t*)_batteryTwoBar);
  }
  else
  {
    u8x8.drawTile(13, 0, 3, (uint8_t*)_batteryBackgroundCharging);
    u8x8.drawTile(14, 0, 1, (uint8_t*)_batteryCharging);
  }

  if (!isBatteryPowered())
  {
    drawTilesFor(0, showPercent ? 9 : 10, 11, (uint8_t*)_emptyTile);
    u8x8.drawTile(12, 0, 1, (uint8_t*)_usbPowered);
  }
  else if (showPercent)
  {
    uint8_t bat = getBatteryLevel();
    if (bat >= 100)
      u8x8.drawString(9, 0, (String(bat) + "%").c_str());
    else if (bat >= 10) 
    {
      u8x8.drawTile(9, 0, 1, (uint8_t*)_emptyTile);
      u8x8.drawString(10, 0, (String(bat) + "%").c_str());
    }
    else 
    {
      u8x8.drawTile(9, 0, 1, (uint8_t*)_emptyTile);
      u8x8.drawTile(10, 0, 1, (uint8_t*)_emptyTile);
      u8x8.drawString(11, 0, (String(bat) + "%").c_str());
    }
  }
  else
    drawTilesFor(0, 9, 12, (uint8_t*)_emptyTile);
}