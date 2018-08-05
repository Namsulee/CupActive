#include <Adafruit_NeoPixel.h>
#include <LedControl.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#include "led.h"
#include "led_action.h"

void SetStripLedOn(Adafruit_NeoPixel *strip, uint32_t c)
{
  for(uint16_t i=0; i < strip->numPixels(); i++) {
      strip->setPixelColor(i, c);
  }
  strip->show();
}

void SetStripLedOff(Adafruit_NeoPixel *strip) 
{
    strip->clear();
    strip->show();  
}

void SetStripLedBrightness(Adafruit_NeoPixel *strip, uint8_t brightness)
{
    strip->setBrightness(brightness);
}

uint32_t ConvertStripLedColor(uint8_t r, uint8_t g, uint8_t b)
{
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

void SetStripLedEffectBlink(Adafruit_NeoPixel *strip, uint32_t c, uint8_t wait)
{
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip->numPixels(); i=i+3) {
        strip->setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip->show();
     
      delay(wait);
     
      for (int i=0; i<strip->numPixels(); i=i+3) {
        strip->setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

void DrawDotMatrix(LedControl *lc, int num) 
{
  if (num == 1) {
    lc->setRow(0,0,one[0]);
    lc->setRow(0,1,one[1]);
    lc->setRow(0,2,one[2]);
    lc->setRow(0,3,one[3]);
    lc->setRow(0,4,one[4]);
    lc->setRow(0,5,one[5]);
    lc->setRow(0,6,one[6]);
    lc->setRow(0,7,one[7]);
  } else if (num == 2) {
    lc->setRow(0,0,two[0]);
    lc->setRow(0,1,two[1]);
    lc->setRow(0,2,two[2]);
    lc->setRow(0,3,two[3]);
    lc->setRow(0,4,two[4]);
    lc->setRow(0,5,two[5]);
    lc->setRow(0,6,two[6]);
    lc->setRow(0,7,two[7]);
  } else if (num == 3) {
    lc->setRow(0,0,three[0]);
    lc->setRow(0,1,three[1]);
    lc->setRow(0,2,three[2]);
    lc->setRow(0,3,three[3]);
    lc->setRow(0,4,three[4]);
    lc->setRow(0,5,three[5]);
    lc->setRow(0,6,three[6]);
    lc->setRow(0,7,three[7]);
  } else if (num == 4) {
    lc->setRow(0,0,four[0]);
    lc->setRow(0,1,four[1]);
    lc->setRow(0,2,four[2]);
    lc->setRow(0,3,four[3]);
    lc->setRow(0,4,four[4]);
    lc->setRow(0,5,four[5]);
    lc->setRow(0,6,four[6]);
    lc->setRow(0,7,four[7]);
  } else if (num == 5) {
    lc->setRow(0,0,five[0]);
    lc->setRow(0,1,five[1]);
    lc->setRow(0,2,five[2]);
    lc->setRow(0,3,five[3]);
    lc->setRow(0,4,five[4]);
    lc->setRow(0,5,five[5]);
    lc->setRow(0,6,five[6]);
    lc->setRow(0,7,five[7]);
  } else if (num == 6) {
    lc->setRow(0,0,six[0]);
    lc->setRow(0,1,six[1]);
    lc->setRow(0,2,six[2]);
    lc->setRow(0,3,six[3]);
    lc->setRow(0,4,six[4]);
    lc->setRow(0,5,six[5]);
    lc->setRow(0,6,six[6]);
    lc->setRow(0,7,six[7]);
  } else if (num == 7) {
    lc->setRow(0,0,seven[0]);
    lc->setRow(0,1,seven[1]);
    lc->setRow(0,2,seven[2]);
    lc->setRow(0,3,seven[3]);
    lc->setRow(0,4,seven[4]);
    lc->setRow(0,5,seven[5]);
    lc->setRow(0,6,seven[6]);
    lc->setRow(0,7,seven[7]);
  } else if (num == 8) {
    lc->setRow(0,0,eight[0]);
    lc->setRow(0,1,eight[1]);
    lc->setRow(0,2,eight[2]);
    lc->setRow(0,3,eight[3]);
    lc->setRow(0,4,eight[4]);
    lc->setRow(0,5,eight[5]);
    lc->setRow(0,6,eight[6]);
    lc->setRow(0,7,eight[7]);
  } else if (num == 9) {
    lc->setRow(0,0,nine[0]);
    lc->setRow(0,1,nine[1]);
    lc->setRow(0,2,nine[2]);
    lc->setRow(0,3,nine[3]);
    lc->setRow(0,4,nine[4]);
    lc->setRow(0,5,nine[5]);
    lc->setRow(0,6,nine[6]);
    lc->setRow(0,7,nine[7]);
  } else {
    lc->setRow(0,0,zero[0]);
    lc->setRow(0,1,zero[1]);
    lc->setRow(0,2,zero[2]);
    lc->setRow(0,3,zero[3]);
    lc->setRow(0,4,zero[4]);
    lc->setRow(0,5,zero[5]);
    lc->setRow(0,6,zero[6]);
    lc->setRow(0,7,zero[7]);
  }
}

void DrawHeartDotMatrix(LedControl *lc) 
{
  lc->setRow(0,0,heart[0]);
  lc->setRow(0,1,heart[1]);
  lc->setRow(0,2,heart[2]);
  lc->setRow(0,3,heart[3]);
  lc->setRow(0,4,heart[4]);
  lc->setRow(0,5,heart[5]);
  lc->setRow(0,6,heart[6]);
  lc->setRow(0,7,heart[7]);
}

void DrawQuestionDotMatrix(LedControl *lc) 
{
  lc->setRow(0,0,question[0]);
  lc->setRow(0,1,question[1]);
  lc->setRow(0,2,question[2]);
  lc->setRow(0,3,question[3]);
  lc->setRow(0,4,question[4]);
  lc->setRow(0,5,question[5]);
  lc->setRow(0,6,question[6]);
  lc->setRow(0,7,question[7]);
}

void DrawSadDotMatrix(LedControl *lc) 
{
  lc->setRow(0,0,sad[0]);
  lc->setRow(0,1,sad[1]);
  lc->setRow(0,2,sad[2]);
  lc->setRow(0,3,sad[3]);
  lc->setRow(0,4,sad[4]);
  lc->setRow(0,5,sad[5]);
  lc->setRow(0,6,sad[6]);
  lc->setRow(0,7,sad[7]);
}

void DrawHappyDotMatrix(LedControl *lc) 
{
  lc->setRow(0,0,happy[0]);
  lc->setRow(0,1,happy[1]);
  lc->setRow(0,2,happy[2]);
  lc->setRow(0,3,happy[3]);
  lc->setRow(0,4,happy[4]);
  lc->setRow(0,5,happy[5]);
  lc->setRow(0,6,happy[6]);
  lc->setRow(0,7,happy[7]);
}
void ClearDotMatrix(LedControl *lc)
{
   lc->clearDisplay(0);
}

