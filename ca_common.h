#ifndef _CA_COMMON_H_
#define _CA_COMMON_H_

#define JM

#if defined(NS)
  #define UNIQUE_ID "NS"
  float CALIBRATION_FACTOR = -2860.0; //This value is obtained using the SparkFun_HX711_Calibration sketch
#elif defined(JM)
  #define UNIQUE_ID "JM"
  float CALIBRATION_FACTOR = -3070.0; //This value is obtained using the SparkFun_HX711_Calibration sketch
#elif defined(YS)
  #define UNIQUE_ID "YS"
  float CALIBRATION_FACTOR = -2860.0; //This value is obtained using the SparkFun_HX711_Calibration sketch
#elif defined(BC)
  #define UNIQUE_ID "BC"
  float CALIBRATION_FACTOR = -2860.0; //This value is obtained using the SparkFun_HX711_Calibration sketch
#else
  #define UNIQUE_ID "JY"
  float CALIBRATION_FACTOR = -2860.0; //This value is obtained using the SparkFun_HX711_Calibration sketch
#endif

#if defined(HOME)
  const char* ssid     = "ynshero-main";
  const char* password = "qa1ws2ed3";
#elif defined(MAKERS)
  const char* ssid     = "RoomC02";
  const char* password = "pwningsdrm";
#else
  const char* ssid     = "soolsang";
  const char* password = "soolsang123";
#endif
char path[] = "/ws";
char host[] = "192.168.0.8";


// define serial speed
#define SERIAL_SPEED 115200
// define LED
#define STRIP_LED_PIN 12
#define STRIP_LED_COUNT 22

// define Load cell
#define LOADCEL_DOUT  2
#define LOADCEL_CLK  0

// define Dot Matrix
#define DOTMATRIX_DIN   13
#define DOTMATRIX_CLK   14
#define DOTMATRIX_CS    15

// define Buzzer pin
#define BUZZER_PIN 16

#define TRUE 1
#define FALSE 0

// define cup weight
#define SOJU_WEIGHT_MIN 60
#define SOJU_WEIGHT_MAX 120

#define BEER_WEIGHT_MIN 260
#define BEER_WEIGHT_MAX 270

#define DEFAULT_CAPABILITY 5
#define MAX_WAITING_STANDALONEMODE 10
#define CRITERIA_DEFAULT_CNT 3


// define variable
#define CONVERT_PIXEL 2.55
typedef enum {
    CA_INIT = 0,
    CA_READY = 1,
    CA_REGISTERING = 2,
    CA_REGISTERED = 3,
    CA_USERSETTING = 4,
    CA_DRINKING = 5,
    CA_GAMEMODE = 6,
    CA_EMPTY = 7,
    CA_ERROR = 8
} CA_STATE;

typedef enum {
    CA_NONE = 0,
    CA_BEER = 1,
    CA_SOJU = 2,
    CA_CUSTOM = 3,
} CA_CUP;

typedef enum {
    CA_GAMENONE = 0,
    CA_RANDOM = 1,
    CA_LOVESHOT = 2,
} CA_GAME;

typedef enum {
    CA_GAMENOTSTART = 0,
    CA_GAMESTART = 1,
    CA_GAMEFINISH = 2,
} CA_GAMESTATE;
#endif /* _CA_COMMON_H_ */
