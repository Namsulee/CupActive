#ifndef _LED_H_
#define _LED_H_
void SetStripLedOn(Adafruit_NeoPixel *strip, uint32_t c);
void SetStripLedOff(Adafruit_NeoPixel *strip);
void SetStripLedBrightness(Adafruit_NeoPixel *strip, uint8_t brightness);
uint32_t ConvertStripLedColor(uint8_t r, uint8_t g, uint8_t b);
void SetStripLedEffectBlink(Adafruit_NeoPixel *strip, uint32_t c, uint8_t wait);
void DrawDotMatrix(LedControl *lc, int num);
void ClearDotMatrix(LedControl *lc);
void DrawHeartDotMatrix(LedControl *lc);
void DrawQuestionDotMatrix(LedControl *lc);
void DrawSadDotMatrix(LedControl *lc);
void DrawHappyDotMatrix(LedControl *lc); 
#endif /* _LED_H_ */
