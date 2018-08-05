#include <Thread.h>
#include <ESP8266WiFi.h>
#include <WebSocketClient.h>
#include <ArduinoJson.h> 
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#include <LedControl.h>
#include <Wire.h>
#include <HX711.h>

#include "Adafruit_DRV2605.h"
#include "binary.h"
#include "musical_notes.h"
#include "led.h"
#include "ca_common.h"

Thread wifiThread = Thread();
// Strip LED Instance
Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIP_LED_COUNT, STRIP_LED_PIN, NEO_GRB + NEO_KHZ800);
// Dot Matrix Instance
LedControl lc = LedControl(DOTMATRIX_DIN, DOTMATRIX_CLK, DOTMATRIX_CS, TRUE);
// LoadCell Instance
HX711 scale(LOADCEL_DOUT, LOADCEL_CLK);
// Vibration motor Instance
Adafruit_DRV2605 haptic;
// WebSocketClient Instance
WebSocketClient webSocketClient;
// Use WiFiClient class to create TCP connections
WiFiClient client;

// Global variable
float gCup_weight;
int gCup;
int gState;
int gCount;
int gFill;
int gCapability;
int gGameKind;
int gDrink;
int gGameState;
int gEmptyCheckCnt;

int getCupState() {
  return gState;
}

void setCupState(int state) {
  int prevState = getCupState();
  if (prevState != state) {
    gState = state;
  }
}

// callback for myThread
void wsReceiveCB(){
  String data;
  char cmd[20] = {0,};
  
  if (client.connected()) {
    webSocketClient.getData(data);
    if (data.length() > 0) {
      // if the message is received and parse to set state correctly
      Serial.print("Received data: ");
      Serial.println(data);
      // parsing
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(data);
      json.printTo(Serial);

      if (json.success()) {
          strcpy(cmd, json["cmd"]);
          Serial.println(cmd);
          if (strcmp(cmd,"connected") == 0) {
            Serial.println("Registered");
            setCupState(CA_REGISTERED);
          } else if (strcmp(cmd,"usersetting") == 0) {
            Serial.println("Usersetting");
             gCapability = json["capability"];
             Serial.println(gCapability);
             gCount = gCapability;
             DrawDotMatrix(&lc, gCapability);
             setCupState(CA_DRINKING);
          } else if (strcmp(cmd,"gamesetting") == 0) {
            Serial.println("gamsetting");
            gGameKind = json["kind"];
            gGameState = json["state"];
            ClearDotMatrix(&lc);
            if (gGameState == CA_GAMESTART) {  
              if (gGameKind == CA_RANDOM) {
                DrawQuestionDotMatrix(&lc);
              } else if (gGameKind == CA_LOVESHOT) {
                DrawHeartDotMatrix(&lc);
              }
              setCupState(CA_GAMEMODE);
            } else if (gGameState == CA_GAMEFINISH) {
              gDrink = json["drink"];
              if (gGameKind == CA_RANDOM) {
                if (gDrink == true) {
                  DrawSadDotMatrix(&lc);
                  vibratorOn(110, 20);
                  seOh(BUZZER_PIN);
                } else {
                  DrawHappyDotMatrix(&lc);
                }
              } else if (gGameKind == CA_LOVESHOT) {
                if (gDrink == true) {
                  DrawHeartDotMatrix(&lc);
                  vibratorOn(110, 20);
                  seOh(BUZZER_PIN);
                } else {
                  ClearDotMatrix(&lc);
                }
              } 
            }
          } else if (strcmp(cmd, "restart") == 0) {
            Serial.println("Restart");
            ClearDotMatrix(&lc);
            setCupState(CA_REGISTERED);
          } else {
            Serial.println("not supported command");
          }
      } else {
        Serial.println("failed to load json config");
      }
    }
  } else {
     Serial.println("Client disconnected.");
     while (1) {
       // Hang on disconnect.
     }
  }
}

void setup() {
  gState = CA_READY;
  gCup_weight = 0;
  gCount = 0;
  gCup = CA_BEER;
  gFill = false;
  gGameState = CA_GAMENOTSTART;
  
  initDevice();
  initWifi();

  //Thread Start
  wifiThread.onRun(wsReceiveCB);
  wifiThread.setInterval(300);
  initAction();
  Serial.println("Start CupActive");
}

void loop() {
    if(wifiThread.shouldRun())
      wifiThread.run();
    
    switch(getCupState()) {
        case CA_READY:
          registerCup(UNIQUE_ID);
        case CA_REGISTERING:
          // Register cup holder to the server
          Serial.println("Waiting to register...");
          break;
        case CA_REGISTERED:
        case CA_USERSETTING:
          break;
        case CA_DRINKING:
          if (gCount > 0 && true == checkEmpty(gCup)) {
              gEmptyCheckCnt++;
              if (gEmptyCheckCnt > 5) {
                // real empty
                gState = CA_EMPTY;
                if (gFill == true) {
                  gFill = false;
                  gCount--;
                  DrawDotMatrix(&lc, gCount);
                }
                emptyAction(gCup);
                gEmptyCheckCnt = 0;
              } else {
                Serial.println("Waiting for the cup whether it is empty or not");
              }
          } else {
            gEmptyCheckCnt = 0;
            if (gCount == 0) {
              // Alert
              SetStripLedBrightness(&strip, 255);
              SetStripLedOn(&strip, ConvertStripLedColor(255, 0, 0));
              // buzzer
              seSiren(BUZZER_PIN); 
            } else {
              float val = getWeight();
              Serial.println(val);       
              int inside = val;
              int cal = 0;
              uint8_t portion = 0;
              if (inside > 0) {
                cal = inside * 2.55f;
                if (cal > 255) {
                  portion = 255;
                } else {
                  portion = cal;
                }
                if (portion > 170) {
                  gFill = true;
                }
                SetStripLedBrightness(&strip, portion);
                SetStripLedOn(&strip, ConvertStripLedColor(0,255,0));
              } else {
                SetStripLedOff(&strip);
              }
            }
          }
          break;
        case CA_GAMEMODE:
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
    
    delay(100);
}

void initDevice(void) {

  // Open Serial port to see the logs
  Serial.begin(SERIAL_SPEED);
  // initialize haptic
  haptic.begin();
  haptic.selectLibrary(1);
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
}
void initAction(void) {
    // Buzzer
    //seR2D2(BUZZER_PIN);
    // Vibration
    vibratorOn(110, 20);
    // LED
    SetStripLedOn(&strip, ConvertStripLedColor(255,255,255));
    delay(1000);
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
  char jsonChar[100] = {0,};
  
  if (getCupState() == CA_READY) {
    setCupState(CA_REGISTERING);
    JsonObject& json = jsonBuffer.createObject();
  
    json["cmd"] = "register";
    json["id"] = id;
    json["ipaddress"] = WiFi.localIP().toString();
    json.printTo(Serial);
    if (client.connected()) {
      json.printTo(jsonChar);
       webSocketClient.sendData(jsonChar);
    } else {
      Serial.println("Client disconnected.");
      while (1) {
        // Hang on disconnect.
      }
    }
  }
}

void emptyAction(int cup) {
    // Buzzer
    //seSiren(BUZZER_PIN);
    // Vibration
    //vibratorOn(100, 20);
    // LED
    switch (cup) {
        case CA_SOJU:
          SetStripLedEffectBlink(&strip, ConvertStripLedColor(0,0,255), 50);
          break;
        case CA_BEER:
          SetStripLedEffectBlink(&strip, ConvertStripLedColor(255,255,0), 50);
          break;
        case CA_CUSTOM:
          SetStripLedEffectBlink(&strip, ConvertStripLedColor(0,255,0), 50);
          break;
        default:
          break;
    }

    SetStripLedOff(&strip);
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

void vibratorOn(int num, int iteration) 
{
   haptic.setWaveform(0, num);
   haptic.setWaveform(1, 0);
   haptic.go();
   delay(1000);
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
