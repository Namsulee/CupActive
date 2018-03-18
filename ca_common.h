#ifndef _CA_COMMON_H_
#define _CA_COMMON_H_

// define serial speed
#define SERIAL_SPEED 9600

// define LED
#define STRIP_LED_PIN 12
#define STRIP_LED_COUNT 26

// define Load cell
#define LOADCEL_DOUT  2
#define LOADCEL_CLK  0

typedef enum {
    CA_INIT = 0,
    CA_READY = 1,
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
