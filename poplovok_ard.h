// Poplovok project
// mkrooted256, 2024

#include "Arduino.h"

#define POPLOVOK_VER 2

// Status LED, output
#define PIN_STATUSLED 5
// Lower sensor, input_pullup, default HIGH, impulse LOW
#define PIN_LOWER_SENSOR 2
// Upper sensor, input_pullup, default HIGH, impulse LOW
#define PIN_UPPER_SENSOR 3
// Relay
#define PIN_PUMP 6
#define PUMP_ON HIGH
#define PUMP_OFF LOW

// Note:  interrupt pins for Nano are 	2, 3

// time to ignore subsequent sensor impulses, in ms
#define SENSOR_DEBOUNCING_TIME 500

// Device state
struct SensorUpdateTime {
    unsigned long lower_sensor;
    unsigned long upper_sensor;
};

//
namespace Log {
    enum Level {
        ERROR=1,
        WARNING,
        LOG,
        VERBOSE,
    };
};

void log(Log::Level level, String msg) {
    unsigned long now = millis();
    String line = (String(now, DEC) + " - ") + (String(int(level)) + " - ") + msg;
    Serial.println(line);
}

