#include <Adafruit_NeoPixel.h>
#include <drv2605.h>
#include <HX711.h>
#include "ca_common.h"
#include "musical_notes.h"


Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIP_LED_COUNT, STRIP_LED_PIN, NEO_GRB + NEO_KHZ800);
DRV2605 haptic;
float calibration_factor = -10500.0; //This value is obtained using the SparkFun_HX711_Calibration sketch
HX711 scale(LOADCEL_DOUT, LOADCEL_CLK);
float gCup_weight;
int gCup;
int gState;

void setup() {
  // Open Serial port to see the logs
  Serial.begin(SERIAL_SPEED);

  // initialize haptic
  if (haptic.init(false, true) != 0) Serial.println("init failed!");
  if (haptic.drv2605_AutoCal() != 0) Serial.println("auto calibration failed!");

  strip.begin();
  
  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare();  //Assuming there is no weight on the scale at start up, reset the scale to 0

  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  Serial.println("Start CupActive");
  initAction();
  
  gState = CA_SEARCHING;
  gCup_weight = 0;
  gCup = CA_NONE;
}

void loop() {
    int cup = 0;
    switch(gState) {
        case CA_SEARCHING:
          // check weight of the cup
          cup = checkCup();
          if (cup != CA_NONE) {
            // cup is detected
            Serial.println("Cup Detected");
            gState = CA_DRINKING;
            cupFoundAction(cup);
          }
          break;
        case CA_DRINKING:
          if (true == checkEmpty(gCup)) {
              gState = CA_EMPTY;
              emptyAction(gCup);
          } 
          break;
        case CA_EMPTY:
          if (false == checkEmpty(gCup)) {
              gState = CA_DRINKING;
          }
          break;
        case CA_ERROR:
          break;
        default:
          break;
    }
    delay(1);
}

void initAction(void) {
    // Buzzer
    //Peripheral_Sound_R2D2(BUZZER_PIN);
    seR2D2(BUZZER_PIN);
    // Vibration
    vibratorOn(110, 20);
    // LED
    colorWipe(strip.Color(255, 255, 0), 50); // yellow color
    delay(2000);
    ledOff();
}

void cupFoundAction(int cup) {
    // Buzzer
    //Peripheral_Sound_Coo(BUZZER_PIN);
    seCoo(BUZZER_PIN);
    // Vibration
    vibratorOn(15, 20);
    // LED
    switch (cup) {
        case CA_SOJU:
          colorWipe(strip.Color(0, 0, 255), 100); // Blue
          Serial.println("Soju");
          break;
        case CA_BEER:
          colorWipe(strip.Color(0, 255, 0), 100); // Green
          Serial.println("Beer");
          break;
        case CA_CUSTOM:
          colorWipe(strip.Color(255, 255, 0), 100); // Yellow
          Serial.println("Custom");
          break;
        default:
          break;
    }
    
    ledOff();
}

void emptyAction(int cup) {
    // Buzzer
    //Peripheral_Sound_Siren(BUZZER_PIN);
    seSiren(BUZZER_PIN);
    // Vibration
    vibratorOn(100, 20);

    // LED
    switch (cup) {
        case CA_SOJU:
          theaterChase(strip.Color(0, 0, 255), 50); // Blue
          break;
        case CA_BEER:
          theaterChase(strip.Color(0, 255, 0), 50); // Green
          break;
        case CA_CUSTOM:
          theaterChase(strip.Color(255, 255, 0), 50); // Yellow
          break;
        default:
          break;
    }

    ledOff();
}

float getWeight(void) {
    float units = 0;
    units = scale.get_units(), 1;
    if (units < 0) {
      units = 0.00;
    }
    units *= CONVERT_GRAM;
    Serial.println(units);
    return units;
}

int checkCup() {
  float val = getWeight();
  int retCup = CA_NONE;

  if (gCup_weight != 0) {
    Serial.println("Cup already selected");
  } else {
    if (val > SOJU_WEIGHT_MIN && val <= 100) {
        // soju
        retCup = CA_SOJU;
        gCup_weight = val;
        gCup = CA_SOJU;
        Serial.println(gCup_weight);
    } else if (val > BEER_WEIGHT_MIN && val <= 300) {
        // beer
        retCup = CA_BEER;
        gCup_weight = val;
        gCup - CA_BEER;
        Serial.println(gCup_weight);
    } else if (val > 300) {
        // custom
        retCup = CA_CUSTOM;
        gCup_weight = val;
        gCup = CA_CUSTOM;
        Serial.println(gCup_weight);
    }
  }

  return retCup;
}

bool checkEmpty(int cup) {
    // set the max weight according to cup
    bool bRet = false;
    float val = getWeight();
    /* soju : (60g ~ 150g) */
    /* beer : (100g ~ 300g) */
    switch (gCup) {
        case CA_SOJU:
          if (val >= SOJU_WEIGHT_MIN && val <= SOJU_WEIGHT_MAX) {
              // check empty
              if (val >= gCup_weight && val < (gCup_weight+EMPTY_WEIGHT_MARGIN)) {
                  bRet = true;
                  Serial.println("Empty Soju Cup");
              } else {
                if ((val - gCup_weight) > 0) {
                  float pixel = (val - gCup_weight) * CONVERT_PIXEL;
                  Serial.println(pixel);
                  for(uint16_t i=0; i < strip.numPixels(); i++) {
                      strip.setPixelColor(i, strip.Color(0, 0, (int)pixel));
                  }
                  strip.show();  
                }
              }
              
          } 
          break;
        case CA_BEER:
          if (val >= BEER_WEIGHT_MIN && val <= BEER_WEIGHT_MAX) {
              // check empty
              if (gCup_weight >= val && gCup_weight < (gCup_weight+EMPTY_WEIGHT_MARGIN)) {
                  bRet = true;
              }
              
          } 
          break;
        case CA_CUSTOM:
          break;
        default:
          break;
    }
    
    return bRet;
}

void ledOff() 
{
    strip.clear();
    strip.show();  
}

void vibratorOn(int num, int iteration) 
{
    for (int i = 0; i < iteration; i++) {
        haptic.drv2605_Play_Waveform(num);  
    }
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

void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { 
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//255가지의 색을 나타내는 함수
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

void beep(int speakerPin, float noteFrequency, long noteDuration) {
  int x;
  // Convert the frequency to microseconds
  float microsecondsPerWave = 1000000 / noteFrequency;
  // Calculate how many milliseconds there are per HIGH/LOW cycles.
  float millisecondsPerCycle = 1000 / (microsecondsPerWave * 2);
  // Multiply noteDuration * number or cycles per millisecond
  float loopTime = noteDuration * millisecondsPerCycle;
  // Play the note for the calculated loopTime.
  for (x = 0; x < loopTime; x++) {
    digitalWrite(speakerPin, HIGH);
    delayMicroseconds(microsecondsPerWave);
    digitalWrite(speakerPin, LOW);
    delayMicroseconds(microsecondsPerWave);
  }
}

void seCoo(int pinNo) {
  int i;

  for (i = 0; i < 150; i = i + 10) {
    beep(pinNo, 1295 - i, 22);
    beep(pinNo, 1295 + i, 22);
  }
}

void seOh(int pinNo) {
  int i;

  for (i = 800; i < 2000; i = i + 100) {
    beep(pinNo, i, 11);
  }
  for (i = 2000; i > 50; i = i - 100) {
    beep(pinNo, i, 11);
  }
}

void seSiren(int pinNo) {
  int i;

  for (i = 1; i < 3; i++) {
    beep(pinNo, 550, 494);
    beep(pinNo, 400, 494);
  }
}

void seR2D2(int pinNo) {
  beep(pinNo, note_A7, 100); // A
  beep(pinNo, note_G7, 100); // G
  beep(pinNo, note_E7, 100); // E
  beep(pinNo, note_C7, 100); // C
  beep(pinNo, note_D7, 100); // D
  beep(pinNo, note_B7, 100); // B
  beep(pinNo, note_F7, 100); // F
  beep(pinNo, note_C8, 100); // C
  beep(pinNo, note_A7, 100); // A
  beep(pinNo, note_G7, 100); // G
  beep(pinNo, note_E7, 100); // E
  beep(pinNo, note_C7, 100); // C
  beep(pinNo, note_D7, 100); // D
  beep(pinNo, note_B7, 100); // B
  beep(pinNo, note_F7, 100); // F
  beep(pinNo, note_C8, 100); // C
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

