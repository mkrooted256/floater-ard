// Poplovok project
// mkrooted256, 2024

#include "Arduino.h"

#define POPLOVOK_VER 3

// Status LED, output
#define PIN_STATUSLED 4 
// Lower sensor, input_pullup, default HIGH, impulse LOW
#define PIN_LOWER_SENSOR 2
// Upper sensor, input_pullup, default HIGH, impulse LOW
#define PIN_UPPER_SENSOR 3
// Relay
#define PIN_PUMP 6
#define PUMP_ON LOW
#define PUMP_OFF HIGH

// Note:  interrupt pins for Nano are 	2, 3

// time to ignore subsequent sensor impulses, in ms
#define SENSOR_DEBOUNCING_TIME 200

// Device state
struct SensorUpdateTime {
    unsigned long lower_sensor;
    unsigned long upper_sensor;
};

//
namespace Log {
    enum Level {
        ERROR=1,
        WARNING=2,
        INFO=3,
        VERBOSE=4,
    };
};

// Levels greater than GLOBAL_LOG_LEVEL will not be compiled.
#define LOG_LEVEL 3

void log(Log::Level level, const String& msg) {
    unsigned long now = millis();
    String line = (String(now, DEC) + " - ") + (String(int(Log::ERROR)) + " - ") + msg;
    Serial.println(line);    
}

#if 1 <= LOG_LEVEL
    #define error(msg) log(Log::ERROR, msg);
#else
    #define error(msg)
#endif
#if 2 <= LOG_LEVEL
    #define warning(msg) log(Log::WARNING, msg);
#else
    #define warning(msg)
#endif
#if 3 <= LOG_LEVEL
    #define info(msg) log(Log::INFO, msg);
#else
    #define info(msg)
#endif
#if 4 <= LOG_LEVEL
    #define verbose(msg) log(Log::VERBOSE, msg);
#else
    #define verbose(msg)
#endif

