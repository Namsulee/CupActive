#ifndef _LED_H_
#define _LED_H_

#define LED_BRIGHTNESS_MAX 255

typedef enum {
    CA_LED_COLOR_NONE = 0,
    CA_LED_COLOR_SKY = 1,
    CA_LED_COLOR_BLUE = 2,
    CA_LED_COLOR_YELLOW = 3,
    CA_LED_COLOR_GREEN = 4,
    CA_LED_COLOR_RED = 5,
    CA_LED_COLOR_WHITE = 6,

    CA_LED_COLOR_MAX,
} LED_COLOR;

typedef struct {
  LED_COLOR color;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} ColorTbl;

void SetStripLedOn(Adafruit_NeoPixel *strip, LED_COLOR color);
void SetStripLedOff(Adafruit_NeoPixel *strip);
void SetStripLedBrightness(Adafruit_NeoPixel *strip, uint8_t brightness);
uint32_t convertStripLedColor(uint8_t red, uint8_t green, uint8_t blue);
void SetStripLedEffectBlink(Adafruit_NeoPixel *strip, LED_COLOR color, int cycle, uint8_t wait);
void DrawDotMatrix(LedControl *lc, int num);
void ClearDotMatrix(LedControl *lc);
void DrawHeartDotMatrix(LedControl *lc);
void DrawQuestionDotMatrix(LedControl *lc);
void DrawSadDotMatrix(LedControl *lc);
void DrawHappyDotMatrix(LedControl *lc); 
void DrawNoneDotMatrix(LedControl *lc);
void DrawflagDotMatrix(LedControl *lc);
void DrawRandomNDotMatrix(LedControl *lc);
void DrawFullDotMatrix(LedControl *lc);
#endif /* _LED_H_ */
