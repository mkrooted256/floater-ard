#include "poplovok_ard.h"


// State
enum State {
    ST_SETUP,
    ST_PUMPING,
    ST_IDLING
};

State globalState = ST_SETUP;
SensorUpdateTime handledImpulseTime{0,0};

// Changed in interrupts
volatile SensorUpdateTime lastImpulseTime{0,0};

// Initialization
void setup_inputs() {
    // init pins
    pinMode(PIN_STATUSLED, OUTPUT);
    pinMode(PIN_LOWER_SENSOR, INPUT_PULLUP);
    pinMode(PIN_UPPER_SENSOR, INPUT_PULLUP);
    pinMode(PIN_PUMP, OUTPUT);

    digitalWrite(PIN_STATUSLED, LOW);
    digitalWrite(PIN_PUMP, PUMP_OFF);

    Serial.begin(9600);
}

// Interrupts
void lowerSensorISR() {
    lastImpulseTime.lower_sensor = millis();
}
void upperSensorISR() {
    lastImpulseTime.upper_sensor = millis();
}

// Main
void setup() {
    setup_inputs();
    
    log(Log::LOG, F("Hello"));

    log(Log::LOG, F("Setting up interrupts"));
    // FALLING because inputs configured as input_pullup, default state is HIGH, and impulse is LOW.
    attachInterrupt(digitalPinToInterrupt(PIN_LOWER_SENSOR), lowerSensorISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_UPPER_SENSOR), upperSensorISR, FALLING);
    
    log(Log::LOG, F("Ready"));
    globalState = ST_IDLING;
}

void applyState(State newState) {
    if (globalState == ST_PUMPING && newState == ST_IDLING) {
        log(Log::LOG, F("ST_PUMPING -> ST_IDLING"));
        log(Log::LOG, F("stop pump"));
        digitalWrite(PIN_PUMP, PUMP_OFF);
        digitalWrite(PIN_STATUSLED, LOW);
    }
    if (globalState == ST_IDLING && newState == ST_PUMPING) {
        log(Log::LOG, F("ST_IDLING -> ST_PUMPING"));
        log(Log::LOG, F("run pump"));
        digitalWrite(PIN_PUMP, PUMP_ON);
        digitalWrite(PIN_STATUSLED, HIGH);
    }

    globalState = newState;
}

void loop() {
    if (globalState == ST_IDLING) 
    {
        if (lastImpulseTime.upper_sensor != handledImpulseTime.upper_sensor) 
        {
            applyState(ST_PUMPING);
            handledImpulseTime.upper_sensor = lastImpulseTime.upper_sensor;
        }
    } 
    else
    if (globalState == ST_PUMPING) 
    {
        if (lastImpulseTime.lower_sensor != handledImpulseTime.lower_sensor) 
        {
            applyState(ST_IDLING);
            handledImpulseTime.lower_sensor = lastImpulseTime.lower_sensor;
        }
    }

}
