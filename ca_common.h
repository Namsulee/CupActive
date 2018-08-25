#ifndef _CA_COMMON_H_
#define _CA_COMMON_H_

#define UNIQUE_ID "namsu"
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

float CALIBRATION_FACTOR = -2920.0; //This value is obtained using the SparkFun_HX711_Calibration sketch

// Wifi information
const char* ssid     = "soolsang";
const char* password = "soolsang123";
char path[] = "/ws";
char host[] = "192.168.1.3";

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
