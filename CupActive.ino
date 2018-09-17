#include <Thread.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 
#include <WebSocketClient.h>
#include <ArduinoJson.h> 
#include <Adafruit_NeoPixel.h>
#include <LedControl.h>
#include <Wire.h>
#include <HX711.h>
#include "Adafruit_DRV2605.h"
#include "binary.h"
#include "led.h"
#include "ca_common.h"

// Wifi Receive thread
Thread wifiThread = Thread();
// Strip LED Global Instance Pointer
Adafruit_NeoPixel *strip;
// Dot Matrix Global Instance Pointer
LedControl *lc;
// LoadCell Global Instance Pointer
HX711 *scale;
// Vibration motor Global Instance Pointer
Adafruit_DRV2605 *haptic;
// WebSocketClient Instance
WebSocketClient *webSocketClient;
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
int gRandom;
int gNoti;


void setup() {
  gState = CA_INIT;
  gCup_weight = 0;
  gCount = 0;
  gCup = CA_BEER;
  gFill = false;
  gGameState = CA_GAMENOTSTART;
  // Open Serial port to see the logs
  Serial.begin(SERIAL_SPEED);
  // Device initialization
  initDevice();
  // Draw None on Dot Matrix
  DrawNoneDotMatrix(lc);
  // Set LED Color as Sky
  SetStripLedBrightness(strip, LED_BRIGHTNESS_MAX);
  SetStripLedOn(strip, CA_LED_COLOR_SKY);
 
  //wifi receive thread setart
  wifiThread.onRun(wsReceiveCB);
  wifiThread.setInterval(300);
      
  Serial.println("Start wifi Setup as STA mode");
  //WiFiManager wifiManager;
  //wifiManager.autoConnect();
  WiFi.begin(ssid, password); 
  int cntConnect = 0;
  while (WiFi.status() != WL_CONNECTED) {
    cntConnect++;
    delay(1000);
    Serial.print(".");
    SetStripLedEffectBlink(strip, CA_LED_COLOR_SKY, 1, 50);
    if (cntConnect == MAX_WAITING_STANDALONEMODE) {
      break;
    }
  }

  if (cntConnect < MAX_WAITING_STANDALONEMODE) {
    Serial.println("establshed wifi setting");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    
    // Check Webserver, If it is not able to connect, it will not start the loop()
    while(true != checkWebServer()) {
      cntConnect++;
      delay(1000);
      Serial.println(".");
      SetStripLedEffectBlink(strip, CA_LED_COLOR_SKY, 1, 50);
      if (cntConnect == MAX_WAITING_STANDALONEMODE) {
        break;
      }
    }
  }
  
  if (cntConnect >= MAX_WAITING_STANDALONEMODE) {
    Serial.println("Standalone mode");
    gCapability = DEFAULT_CAPABILITY;
    gCount = DEFAULT_CAPABILITY;
    DrawDotMatrix(lc, gCapability);
    initAction();
    setCupState(CA_DRINKING);
  } else {
    setCupState(CA_READY);
  }
  
  callibrateScale();
  Serial.println("Start CupActive");
}

void loop() {
    // Check Wifi Receive Thread
    if(wifiThread.shouldRun())
      wifiThread.run();
    
    // State machine to work according to Device State
    switch(getCupState()) {
        case CA_INIT:
          break;
        case CA_READY:
          registerCup(UNIQUE_ID);
          SetStripLedEffectBlink(strip, CA_LED_COLOR_SKY, 1, 50);
          break;
        case CA_REGISTERING:
          // Register cup holder to the server
          Serial.println("Waiting to register...");
          SetStripLedEffectBlink(strip, CA_LED_COLOR_SKY, 1, 50);
          break;
        case CA_REGISTERED:
        case CA_USERSETTING:
          // Nothing..
          SetStripLedEffectBlink(strip, CA_LED_COLOR_SKY, 1, 50);
          break;
        case CA_DRINKING:
          if (gCount > 0 && true == checkEmpty(gCup)) {
              gEmptyCheckCnt++;
              if (gEmptyCheckCnt > CRITERIA_DEFAULT_CNT) {
                // real empty
                gState = CA_EMPTY;
                if (gFill == true) {
                  gFill = false;
                  gCount--;
                  DrawDotMatrix(lc, gCount);
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
              if (gNoti == false) {
                SetStripLedBrightness(strip, LED_BRIGHTNESS_MAX);
                SetStripLedOn(strip, CA_LED_COLOR_RED);
                seOh();
                delay(100);
                DrawSadDotMatrix(lc);
                seOh();
                delay(100);
                seOh();
                delay(100);
                gNoti = true;
              }
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
                if (portion > 5) {
                  gFill = true;
                }
                SetStripLedBrightness(strip, portion);
                SetStripLedOn(strip, CA_LED_COLOR_GREEN);
              } else {
                SetStripLedOff(strip);
              }
            }
          }
          break;
        case CA_GAMEMODE:
          // If gCount is over 0, you can join the game
          if (gCount > 0) {
            if (gGameState == CA_GAMENOTSTART) {
              SetStripLedBrightness(strip, LED_BRIGHTNESS_MAX);
              SetStripLedOn(strip, CA_LED_COLOR_WHITE);
            } else if (gGameState == CA_GAMESTART) {
              if (gRandom == true) {
                gRandom = false;
                seOh();
                DrawFullDotMatrix(lc);
              } else {
                gRandom = true;
                ClearDotMatrix(lc);
              }
              SetStripLedEffectBlink(strip, CA_LED_COLOR_WHITE, 3, 50);
            } else if (gGameState == CA_GAMEFINISH) {
              // check cup and show the your reamin count
              if (gDrink == 0) {
                setCupState(CA_DRINKING);
                DrawDotMatrix(lc, gCount);
                SetStripLedOff(strip);
              } else {
                float val = getWeight();
                if (val > -5) {
                  // nothing..cup is still on the soolsang
                } else {
                  SetStripLedOff(strip);
                  DrawDotMatrix(lc, gCount);
                  gGameState = CA_GAMENOTSTART;
                  setCupState(CA_DRINKING);
                  gDrink = 0;
                }
              }
            }
            gNoti = false;
          } else {
            // your gCount is 0, you can not join the game
             if (gNoti == false) {
                SetStripLedBrightness(strip, LED_BRIGHTNESS_MAX);
                SetStripLedOn(strip, CA_LED_COLOR_RED);
                seOh();
                delay(100);
                DrawSadDotMatrix(lc);
                seOh();
                delay(100);
                seOh();
                delay(100);
                gNoti = true;
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
 
  delay(100);
}

void initDevice(void) 
{
  // Allocation memory to use each instances
  strip = new Adafruit_NeoPixel(STRIP_LED_COUNT, STRIP_LED_PIN, NEO_GRB + NEO_KHZ800);
  lc = new LedControl(DOTMATRIX_DIN, DOTMATRIX_CLK, DOTMATRIX_CS, TRUE);
  scale = new HX711(LOADCEL_DOUT, LOADCEL_CLK);
  haptic = new Adafruit_DRV2605();
  webSocketClient = new WebSocketClient();
  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  // Haptic Motor Initialize
  haptic->begin();
  haptic->selectLibrary(1);
  haptic->setMode(DRV2605_MODE_INTTRIG);
  // Strip LED Initialize
  strip->begin();

  // Dot Matrix
  lc->shutdown(0,false);
  // Set brightness to a medium value
  lc->setIntensity(0,8);
  // Clear the display
  lc->clearDisplay(0);  
}

void callibrateScale(void)
{
  // Load cell Initialize
  scale->set_scale(CALIBRATION_FACTOR); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale->tare();  //Assuming there is no weight on the scale at start up, reset the scale to 0
  long zero_factor = scale->read_average(); //Get a baseline reading
  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor); 
}

bool checkWebServer(void) 
{
  // Connect to the websocket server
  if (client.connect(host, 8080)) {
    Serial.println("Connected");
  } else {
    Serial.println("Connection failed.");
    return false;
  }
  // Handshake with the server
  webSocketClient->path = path;
  webSocketClient->host = host;
  if (webSocketClient->handshake(client)) {
    Serial.println("Handshake successful");
  } else {
    Serial.println("Handshake failed.");
    return false;
  }

  return true;
}

void initAction(void) 
{
  // Vibration
  vibratorOn(16, 1);
  // LED
   // Set LED Color as blue
  SetStripLedBrightness(strip, LED_BRIGHTNESS_MAX);
  SetStripLedOn(strip, CA_LED_COLOR_BLUE);
  delay(2000);
}
int getCupState() {
  return gState;
}

void setCupState(int state) 
{
  int prevState = getCupState();
  if (prevState != state) {
    gState = state;
  }
}

// callback for websocket receivee
void wsReceiveCB()
{
  String data;
  char cmd[20] = {0,};

  if (getCupState() < CA_READY) {
    // showing action(waiting...)
    Serial.print(".");
    SetStripLedEffectBlink(strip, CA_LED_COLOR_SKY, 20, 100);
  } else {
      if (client.connected()) {
      webSocketClient->getData(data);
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
               DrawDotMatrix(lc, gCapability);
               // Doing Init action
               initAction();
               setCupState(CA_DRINKING);
            } else if (strcmp(cmd,"gamesetting") == 0) {
              Serial.println("gamsetting");
              setCupState(CA_GAMEMODE);
              gGameKind = json["kind"];
              gGameState = json["state"];
              if (gGameState == CA_GAMENOTSTART){
                Serial.println("Game Ready");
                DrawflagDotMatrix(lc);
              } else if (gGameState == CA_GAMESTART) {
                 // nothing
                 Serial.println("Game State");
              } else if (gGameState == CA_GAMEFINISH) {
                Serial.println("Game Finish");
                gDrink = json["drink"];
                if (gGameKind == CA_RANDOM) {
                  if (gDrink == 1) {
                    DrawRandomNDotMatrix(lc);
                    vibratorOn(16, 5);
                    SetStripLedBrightness(strip, LED_BRIGHTNESS_MAX);
                    SetStripLedOn(strip, CA_LED_COLOR_WHITE);
                    seSiren();
                  } else {
                    DrawHappyDotMatrix(lc);
                    SetStripLedOn(strip, CA_LED_COLOR_NONE);
                    setCupState(CA_DRINKING);
                  }
                } else if (gGameKind == CA_LOVESHOT) {
                  if (gDrink == 1) {
                    DrawHeartDotMatrix(lc);
                    vibratorOn(16, 5);
                    SetStripLedBrightness(strip, LED_BRIGHTNESS_MAX);
                    SetStripLedOn(strip, CA_LED_COLOR_WHITE);
                  } else {
                    ClearDotMatrix(lc);
                    SetStripLedOn(strip, CA_LED_COLOR_NONE);
                  }
                }
              }
            } else if (strcmp(cmd, "restart") == 0) {
              Serial.println("Restart");
              ClearDotMatrix(lc);
              setCupState(CA_REGISTERED);
              gNoti = false;
            } else {
              Serial.println("not supported command");
            }
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  }
}

void registerCup(String id) 
{
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
       webSocketClient->sendData(jsonChar);
    } else {
      Serial.println("Client disconnected.");
    }
  }
}

void emptyAction(int cup) 
{
  // Vibration
  vibratorOn(16, 5);
  seCoo();
  // LED
  switch (cup) {
      case CA_BEER:
        SetStripLedBrightness(strip, LED_BRIGHTNESS_MAX);
        SetStripLedOn(strip, CA_LED_COLOR_YELLOW);
        break;
      default:
        break;
  }
}

float getWeight(void) 
{
  float units = 0;
  units = scale->get_units(), 1;
  Serial.print(units);
  Serial.println();
  return units;
}

bool checkEmpty(int cup) 
{
  // set the max weight according to cup
  bool bRet = false;
  float val = getWeight();
  switch (gCup) {
    case CA_BEER:
        if (val >= -5 && val <= 5) {
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

  delay(500);
  return bRet;
}

void vibratorOn(int num, int iteration) 
{
  for (int i = iteration; i <= iteration; i++) {
   haptic->setWaveform(0, num);
   haptic->setWaveform(1, 0);
   haptic->go();
   delay(1000);
  }
}

void beep(int speakerPin, float noteFrequency, long noteDuration) 
{
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

void seSiren(void) 
{
  int i;

  for (i = 1; i < 3; i++) {
    beep(BUZZER_PIN, 550, 494);
    beep(BUZZER_PIN, 400, 494);
  }
}

void seOh(void) 
{
  int i;

  for (i = 800; i < 2000; i = i + 100) {
    beep(BUZZER_PIN, i, 11);
  }
  for (i = 2000; i > 50; i = i - 100) {
    beep(BUZZER_PIN, i, 11);
  }
}

void seCoo(void) 
{
  int i;

  for (i = 0; i < 150; i = i + 10) {
    beep(BUZZER_PIN, 1295 - i, 22);
    beep(BUZZER_PIN, 1295 + i, 22);
  }
}

