// Poplovok project
// mkrooted256, 2024

#include "Arduino.h"

#define POPLOVOK_VER 1 

// Status LED, output
#define PIN_STATUSLED 5
// Lower sensor, input_pullup, default HIGH, impulse LOW
#define PIN_LOWER_SENSOR 2
// Upper sensor, input_pullup, default HIGH, impulse LOW
#define PIN_UPPER_SENSOR 3

// Note:  interrupt pins for Nano are 	2, 3

// time to wait for serial input before entering production mode, in ms
#define SERIAL_INIT_TIME 3000
// time to ignore subsequent sensor impulses, in ms
#define SENSOR_DEBOUNCING_TIME 500
// check if sensors are connected if no impulse for this period, in ms
#define SENSORS_CHECK_INTERVAL 30000

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

// Signaling
namespace Signaling {
    enum Signal {
        O_INIT=0,
        O_ST_NOT_INITIALIZED,
        O_ST_IDLING,
        O_ST_PUMPING,
        O_ERROR
    };
    
    struct BlinkSequence {
        const unsigned long beatDuration; // ms
        const unsigned int len; // len of sequence
        const char* sequence; // '1'/'0' sequence
    };

    const BlinkSequence seq_init{1000, 1, "1"};
    const BlinkSequence seq_not_initialized{125, 10, "1010100000"};
    const BlinkSequence seq_idling{125, 8, "10000000"};
    const BlinkSequence seq_pumping{125, 8, "11111100"};
    const BlinkSequence seq_error{500, 2, "10"};

    struct Blinker {
        const BlinkSequence* currentSequence = nullptr;
        unsigned int beatNumber = 0;
        unsigned long lastUpdateTime = 0;

        void play(const BlinkSequence* seq) {
            currentSequence = seq;
            beatNumber = 0;
            lastUpdateTime = 0;
            bool nextLevel = HIGH ? (currentSequence->sequence[0] == '1') : LOW;
            digitalWrite(PIN_STATUSLED, nextLevel);
            log(Log::VERBOSE, F("blinker::play"));
        }

        void stop() {
            digitalWrite(PIN_STATUSLED, LOW);
            currentSequence = nullptr;
            log(Log::VERBOSE, F("blinker::stop"));
        }

        void advanceBeat() {
            beatNumber = (beatNumber + 1) % currentSequence->len;
            bool nextLevel = HIGH ? (currentSequence->sequence[beatNumber] == '1') : LOW;
            digitalWrite(PIN_STATUSLED, nextLevel);
        }

        void loop() {
            if (!currentSequence) return;
            if (millis() - lastUpdateTime >= currentSequence->beatDuration) {
                advanceBeat();
                lastUpdateTime = millis();
            }
        }
    };
};

//
namespace NormalMode {
    enum State {
        // No impulses yet
        ST_NOT_INITIALIZED = 0,
        ST_IDLING,
        ST_PUMPING,
    };
    enum Sensor {
        SEN_UPPER = 1,
        SEN_LOWER,
    };

    struct Mode {
        State state = ST_NOT_INITIALIZED;
        Signaling::Blinker blinker;
        SensorUpdateTime handledImpulseTimes{0,0};
        volatile SensorUpdateTime lastImpulseTimes{0,0};

        // blocking!
        // inputs should be HIGH for ~500ms straight.
        bool verifySensorsConnected() {
            for (int i=0; i<50; i++) {
                delay(10);
                if (digitalRead(PIN_LOWER_SENSOR) == LOW) return false;
                if (digitalRead(PIN_UPPER_SENSOR) == LOW) return false;
            }
            return true;
        }

        void initialize() {
            if (state == ST_NOT_INITIALIZED) {
                if (!verifySensorsConnected()) {
                    blinker.play(&Signaling::seq_error);
                    log(Log::WARNING, F("Failed to initialize NormalMode - sensors are disconnected. ST_NOT_INITIALIZED."));
                    return;
                }

                state = ST_IDLING;
                blinker.play(&Signaling::seq_idling);
                log(Log::LOG, F("Initializing NormalMode. ST_IDLING."));
            }
        }

        void triggerLowerSensor() {
            if (state == ST_PUMPING) {
                state = ST_IDLING;
                blinker.play(&Signaling::seq_idling);
                log(Log::LOG, F("triggerLowerSensor - to ST_IDLING"));
            }
        }

        void triggerUpperSensor() {
            if (state == ST_IDLING) {
                state = ST_PUMPING;
                blinker.play(&Signaling::seq_pumping);
                log(Log::LOG, F("triggerUpperSensor - to ST_PUMPING"));
            }
        }

        void loop() {
            // To maintain const value throughout whole function (lastImpulseTimes is volatile)
            SensorUpdateTime cur_lastImpulseTimes{lastImpulseTimes.lower_sensor, lastImpulseTimes.upper_sensor}; 
            bool impulseDetected = false;

            if (cur_lastImpulseTimes.lower_sensor - handledImpulseTimes.lower_sensor > SENSOR_DEBOUNCING_TIME) {
                triggerLowerSensor();
                handledImpulseTimes.lower_sensor = cur_lastImpulseTimes.lower_sensor;
                impulseDetected = true;
            }
            if (cur_lastImpulseTimes.upper_sensor - handledImpulseTimes.upper_sensor > SENSOR_DEBOUNCING_TIME) {
                triggerUpperSensor();
                handledImpulseTimes.upper_sensor = cur_lastImpulseTimes.upper_sensor;
                impulseDetected = true;
            }

            blinker.loop();
        }
    };
};
