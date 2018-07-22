#ifndef _CA_COMMON_H_
#define _CA_COMMON_H_

#define UNIQUE_ID "hero"
// define serial speed
#define SERIAL_SPEED 9600

// define LED
#define STRIP_LED_PIN 12
#define STRIP_LED_COUNT 22

// define Load cell
#define LOADCEL_DOUT  2
#define LOADCEL_CLK  0

// define buzzer
#define BUZZER_PIN 16

// define Dot Matrix
#define DOTMATRIX_DIN   13
#define DOTMATRIX_CLK   14
#define DOTMATRIX_CS    15

#define TRUE 1
#define FALSE 0

// define cup weight
#define SOJU_WEIGHT_MIN 60
#define SOJU_WEIGHT_MAX 120

#define BEER_WEIGHT_MIN 260
#define BEER_WEIGHT_MAX 270

float CALIBRATION_FACTOR = -2820.0; //This value is obtained using the SparkFun_HX711_Calibration sketch

// Wifi information
const char* ssid     = "ynshero-main";
const char* password = "qa1ws2ed3";
char path[] = "/ws";
char host[] = "192.168.0.8";

// define variable
#define CONVERT_PIXEL 2.55
typedef enum {
    CA_REGISTERING = 0,
    CA_USERSETTING = 1,
    CA_DRINKING = 2,
    CA_EMPTY = 3,
    CA_ERROR = 4
} CA_STATE;

typedef enum {
    CA_NONE = 0,
    CA_BEER = 1,
    CA_SOJU = 2,
    CA_CUSTOM = 3,
} CA_CUP;

#endif /* _CA_COMMON_H_ */
