//#include <MyLedLib.h>

#include "ca_common.h"
#include <Adafruit_NeoPixel.h>
#include <drv2605.h>
#include <HX711.h>


Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIP_LED_COUNT, STRIP_LED_PIN, NEO_GRB + NEO_KHZ800);
// define vibrator motoer
DRV2605 haptic;
// define Load cell
float calibration_factor = -7050.0; //This value is obtained using the SparkFun_HX711_Calibration sketch
HX711 scale(LOADCEL_DOUT, LOADCEL_CLK);
float units;
float gCup_weight;
int gFillCount;
int gState; 

void setup() {
  // Open Serial port to see the logs
  Serial.begin(SERIAL_SPEED);

  // Create MyLedLib Instance
  //led = new MyLedLib(STRIP_LED_PIN, STRIP_LED_COUNT);
  strip.begin();
  strip.show();

  // initialize haptic
  if (haptic.init(false, true) != 0) Serial.println("init failed!");
  if (haptic.drv2605_AutoCal() != 0) Serial.println("auto calibration failed!");

  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare();  //Assuming there is no weight on the scale at start up, reset the scale to 0
  
  Serial.println("Start CupActive");
}

float getWeight(void) {
    units = scale.get_units(), 1;
    if (units < 0) {
      units = 0.00;
    }
    Serial.print(units);
    Serial.println(" lbs");
    return units;
}

int checkCup() {
  float val = getWeight();
  int retCup = CA_NONE;
  gCup_weight = val;
  
  if (val > 10 && val <= 100) {
      // soju
      retCup = CA_SOJU;
  } else if (val > 100 && val <= 400) {
      // beer
      retCup = CA_BEER;
  } else if (val > 400) {
      // custom
      retCup = CA_CUSTOM;
  } else {
      // Cup is not detected
  }

  return retCup;
}

void actCupFind(int cup) {
    // led and motor
    haptic.drv2605_Play_Waveform(16);
    switch (cup) {
        case CA_SOJU:
          colorWipe(strip.Color(0, 0, 255), 100); //파란색 출력
          break;
        case CA_BEER:
          colorWipe(strip.Color(0, 255, 0), 100); //녹색 출력
          break;
        case CA_CUSTOM:
          colorWipe(strip.Color(255, 255, 0), 100); 
          break;
    }
    
    strip.clear();
    strip.begin();
    strip.show();
}

int fillInCup(int cup) {
    // set the max weight according to cup
    float val = getWeight();

    if (val == gCup_weight && gFillCount < 10) {
        gFillCount++;
    } else {
        gFillCount = 0;
    }

    if (gFillCount >= 10) {
        return 1;
    }

    return 0;
}

void finish() {
  gState = CA_INIT; 
}
void loop() {
    int cup = 0;
    
    switch(gState) {
        case CA_INIT:
          // check weight of the cup
          //led->EffectOn(EFFECT_BLINKING, COLOR_WHITE, 100);
          strip.setBrightness(50);
          theaterChase(strip.Color(127, 127, 127), 50); // white color
          cup = checkCup();
          if (cup != CA_NONE) {
            // cup is detected
            Serial.print("weight ");
            Serial.println(gCup_weight);
            gState = CA_READY;
            actCupFind(cup);
          }
          break;
        case CA_READY:
          if (1 == fillInCup(cup)) {
              gState = CA_DRINKING;
          }
          break;
        case CA_DRINKING:
          break;
        case CA_EMPTY:
          break;
        case CA_ERROR:
          break;
        default:
          break;
    }
    delay(1);
}

// LED custom functions
//입력한 색으로 LED를 깜빡거리며 표현한다
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();
     
      delay(wait);
     
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

/*
 

#include "HX711.h"

#define DOUT  2
#define CLK  0

HX711 scale(DOUT, CLK);

float calibration_factor = -7050; //-7050 worked for my 440lb max scale setup

void setup() {
  Serial.begin(9600);
  Serial.println("HX711 calibration sketch");
  Serial.println("Remove all weight from scale");
  Serial.println("After readings begin, place known weight on scale");
  Serial.println("Press + or a to increase calibration factor");
  Serial.println("Press - or z to decrease calibration factor");

  scale.set_scale();
  scale.tare(); //Reset the scale to 0

  long zero_factor = scale.read_average(); //Get a baseline reading
  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor);
}

void loop() {

  scale.set_scale(calibration_factor); //Adjust to this calibration factor

  Serial.print("Reading: ");
  Serial.print(scale.get_units(), 1);
  Serial.print(" lbs"); //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
  Serial.print(" calibration_factor: ");
  Serial.print(calibration_factor);
  Serial.println();

  if(Serial.available())
  {
    char temp = Serial.read();
    if(temp == '+' || temp == 'a')
      calibration_factor += 10;
    else if(temp == '-' || temp == 'z')
      calibration_factor -= 10;
  }
}
*/

