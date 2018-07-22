#include <ESP8266WiFi.h>
#include <WebSocketClient.h>
#include <ArduinoJson.h> 
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#include <LedControl.h>
#include <Wire.h>
#include <Adafruit_DRV2605.h>
#include <HX711.h>
#include "binary.h"
#include "musical_notes.h"
#include "led_action.h"
#include "ca_common.h"

Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIP_LED_COUNT, STRIP_LED_PIN, NEO_GRB + NEO_KHZ800);
LedControl lc = LedControl(DOTMATRIX_DIN, DOTMATRIX_CLK, DOTMATRIX_CS, TRUE);
HX711 scale(LOADCEL_DOUT, LOADCEL_CLK);
Adafruit_DRV2605 haptic;  
WebSocketClient webSocketClient;
// Use WiFiClient class to create TCP connections
WiFiClient client;

float gCup_weight;
int gCup;
int gState;
int gCount;
int gFill;
int gCapability;

void setup() {
  gState = CA_REGISTERING;
  gCup_weight = 0;
  gCount = 0;
  gCup = CA_BEER;
  gFill = false;
  
  initDevice();
  initAction();
  initWifi();

  Serial.println("Start CupActive");
}

void loop() {

    switch(gState) {
        case CA_REGISTERING:
          // Register cup holder to the server
          registerCup(UNIQUE_ID);
          break;
        case CA_USERSETTING:
          setUserCapability();
          break;
        case CA_DRINKING:
          if (true == checkEmpty(gCup)) {
              gState = CA_EMPTY;
              if (gFill == true) {
                gFill = false;
                gCount++;
                draw(gCount);
              }
              emptyAction(gCup);
          } else {
            if (gCount > gCapability) {
              // Alert
              strip.setBrightness(255);
              ledOn(strip.Color(255, 0, 0));
            } else {
              float val = getWeight();
              Serial.print(val);
              Serial.println();
          
              int inside = val;
              int cal = 0;
              uint8_t portion = 0;
              if (inside > 0) {
                //Serial.println(inside*2.55);
                cal = inside * 2.55f;
                if (cal > 255) {
                  portion = 255;
                } else {
                  portion = cal;
                }
                if (portion > 170) {
                  gFill = true;
                }
                strip.setBrightness(portion);
                ledOn(strip.Color(0, 255, 0)); // Green
              } else {
                ledOff();
              }
            }
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

int getCupState() {
  return gState;
}

void setCupState(int state) {
  int prevState = getCupState();
  if (prevState != state) {
    gState = state;
  }
}
void initDevice(void) {

  // Open Serial port to see the logs
  Serial.begin(SERIAL_SPEED);
  
  // initialize haptic
  //if (haptic.init(false, true) != 0) Serial.println("init failed!");
  //if (haptic.drv2605_AutoCal() != 0) Serial.println("auto calibration failed!");
  haptic.begin();
  haptic.setMode(DRV2605_MODE_INTTRIG); 
   
  // Strip LED Initialize
  strip.begin();

  // Load cell Initialize
  scale.set_scale(CALIBRATION_FACTOR); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare();  //Assuming there is no weight on the scale at start up, reset the scale to 0
  long zero_factor = scale.read_average(); //Get a baseline reading
  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor);
  
  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);

  // Dot Matrix
  lc.shutdown(0,false);
  // Set brightness to a medium value
  lc.setIntensity(0,8);
  // Clear the display
  lc.clearDisplay(0);  

  draw(0);
}
void initAction(void) {
    // Buzzer
    //seR2D2(BUZZER_PIN);
    // Vibration
    //vibratorOn(110, 20);
    // LED
    ledOn(strip.Color(255, 255, 255)); // white color
    delay(2000);
}

void initWifi(void) {
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  delay(5000);
  

  // Connect to the websocket server
  if (client.connect("192.168.0.8", 8080)) {
    Serial.println("Connected");
  } else {
    Serial.println("Connection failed.");
    while(1) {
      // Hang on failure
    }
  }

  // Handshake with the server
  webSocketClient.path = path;
  webSocketClient.host = host;
  if (webSocketClient.handshake(client)) {
    Serial.println("Handshake successful");
  } else {
    Serial.println("Handshake failed.");
    while(1) {
      // Hang on failure
    }  
  }
}

void registerCup(String id) {
  DynamicJsonBuffer jsonBuffer;
  char jsonChar[100];
  
  if (getCupState() == CA_REGISTERING) {
    JsonObject& json = jsonBuffer.createObject();
  
    json["cmd"] = "register";
    json["id"] = id;
    json.printTo(Serial);
    if (client.connected()) {
      json.printTo(jsonChar);
       webSocketClient.sendData(jsonChar);
       setCupState(CA_USERSETTING);
    } else {
      Serial.println("Client disconnected.");
      while (1) {
        // Hang on disconnect.
      }
    }
  }
}

void setUserCapability() {
  String data;
  char cmd[20];
  
  if (client.connected()) {
    webSocketClient.getData(data);
    if (data.length() > 0) {
      Serial.print("Received data: ");
      Serial.println(data); 
      // parsing
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(data);
      json.printTo(Serial);

      if (json.success()) {
          Serial.println("\nparsed json");
          strcpy(cmd, json["cmd"]);
          if (cmd == "usersetting") {
            gCapability = json["capability"];
            setCupState(CA_DRINKING);
          }
      } else {
        Serial.println("failed to load json config");
      }
    } else {
      Serial.print("data is 0");
      Serial.println();
    }
  } else {
    Serial.println("Client disconnected.");
    while (1) {
      // Hang on disconnect.
    }
  }
}

void receiveDataWS() {
  String data;
    
  if (client.connected()) {
    webSocketClient.getData(data);
    if (data.length() > 0) {
      Serial.print("Received data: ");
      Serial.println(data);
    }
  } else {
    Serial.println("Client disconnected.");
    while (1) {
      // Hang on disconnect.
    }
  }
}

void sendDataWS() {
  String data;
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
   
  if (client.connected()) {
    data = "hello world";
    webSocketClient.sendData(data);
 
  } else {
    Serial.println("Client disconnected.");
    while (1) {
      // Hang on disconnect.
    }
  }
}

void cupFoundAction(int cup) {
    // Buzzer
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
          ledOn(strip.Color(0, 255, 0));
          //colorWipe(strip.Color(0, 255, 0), 100); // Green
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
    //seSiren(BUZZER_PIN);
    // Vibration
    //vibratorOn(100, 20);

    // LED
    switch (cup) {
        case CA_SOJU:
          theaterChase(strip.Color(0, 0, 255), 50); // Blue
          break;
        case CA_BEER:
          theaterChase(strip.Color(255, 255, 0), 50); // yellow
          break;
        case CA_CUSTOM:
          theaterChase(strip.Color(255, 255, 0), 50); // Yellow
          break;
        default:
          break;
    }

    //ledOff();
}

float getWeight(void) {
    float units = 0;
    units = scale.get_units(), 1;
    if (units < 0) {
      units = 0.00;
    }
   
    Serial.print(units);
    Serial.println();
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
        case CA_BEER:
            if (val >= 0 && val <= 5) {
              // check empty
              bRet = true;
            } else {
              bRet = false; 
            }
          break;
        case CA_CUSTOM:
          break;
        default:
          break;
    }
    
    return bRet;
}

void ledOn(uint32_t c)
{
  for(uint16_t i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
  }
  strip.show();
}

void ledOff() 
{
    strip.clear();
    strip.show();  
}

void draw(int num) {
  if (num == 1) {
    lc.setRow(0,0,one[0]);
    lc.setRow(0,1,one[1]);
    lc.setRow(0,2,one[2]);
    lc.setRow(0,3,one[3]);
    lc.setRow(0,4,one[4]);
    lc.setRow(0,5,one[5]);
    lc.setRow(0,6,one[6]);
    lc.setRow(0,7,one[7]);
  } else if (num == 2) {
    lc.setRow(0,0,two[0]);
    lc.setRow(0,1,two[1]);
    lc.setRow(0,2,two[2]);
    lc.setRow(0,3,two[3]);
    lc.setRow(0,4,two[4]);
    lc.setRow(0,5,two[5]);
    lc.setRow(0,6,two[6]);
    lc.setRow(0,7,two[7]);
  } else if (num == 3) {
    lc.setRow(0,0,three[0]);
    lc.setRow(0,1,three[1]);
    lc.setRow(0,2,three[2]);
    lc.setRow(0,3,three[3]);
    lc.setRow(0,4,three[4]);
    lc.setRow(0,5,three[5]);
    lc.setRow(0,6,three[6]);
    lc.setRow(0,7,three[7]);
  } else if (num == 4) {
    lc.setRow(0,0,four[0]);
    lc.setRow(0,1,four[1]);
    lc.setRow(0,2,four[2]);
    lc.setRow(0,3,four[3]);
    lc.setRow(0,4,four[4]);
    lc.setRow(0,5,four[5]);
    lc.setRow(0,6,four[6]);
    lc.setRow(0,7,four[7]);
  } else if (num == 5) {
    lc.setRow(0,0,five[0]);
    lc.setRow(0,1,five[1]);
    lc.setRow(0,2,five[2]);
    lc.setRow(0,3,five[3]);
    lc.setRow(0,4,five[4]);
    lc.setRow(0,5,five[5]);
    lc.setRow(0,6,five[6]);
    lc.setRow(0,7,five[7]);
  } else if (num == 6) {
    lc.setRow(0,0,six[0]);
    lc.setRow(0,1,six[1]);
    lc.setRow(0,2,six[2]);
    lc.setRow(0,3,six[3]);
    lc.setRow(0,4,six[4]);
    lc.setRow(0,5,six[5]);
    lc.setRow(0,6,six[6]);
    lc.setRow(0,7,six[7]);
  } else if (num == 7) {
    lc.setRow(0,0,seven[0]);
    lc.setRow(0,1,seven[1]);
    lc.setRow(0,2,seven[2]);
    lc.setRow(0,3,seven[3]);
    lc.setRow(0,4,seven[4]);
    lc.setRow(0,5,seven[5]);
    lc.setRow(0,6,seven[6]);
    lc.setRow(0,7,seven[7]);
  } else if (num == 8) {
    lc.setRow(0,0,eight[0]);
    lc.setRow(0,1,eight[1]);
    lc.setRow(0,2,eight[2]);
    lc.setRow(0,3,eight[3]);
    lc.setRow(0,4,eight[4]);
    lc.setRow(0,5,eight[5]);
    lc.setRow(0,6,eight[6]);
    lc.setRow(0,7,eight[7]);
  } else if (num == 9) {
    lc.setRow(0,0,nine[0]);
    lc.setRow(0,1,nine[1]);
    lc.setRow(0,2,nine[2]);
    lc.setRow(0,3,nine[3]);
    lc.setRow(0,4,nine[4]);
    lc.setRow(0,5,nine[5]);
    lc.setRow(0,6,nine[6]);
    lc.setRow(0,7,nine[7]);
  } else {
    lc.setRow(0,0,zero[0]);
    lc.setRow(0,1,zero[1]);
    lc.setRow(0,2,zero[2]);
    lc.setRow(0,3,zero[3]);
    lc.setRow(0,4,zero[4]);
    lc.setRow(0,5,zero[5]);
    lc.setRow(0,6,zero[6]);
    lc.setRow(0,7,zero[7]);
  }
}

void vibratorOn(int num, int iteration) 
{
    haptic.setWaveform(0, num);
    haptic.setWaveform(1, 0);

    haptic.go();
    delay(1000);
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
