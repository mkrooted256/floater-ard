#include "poplovok_ard.h"


// State
NormalMode::Mode mode;

// Initialization
void setup_inputs() {
    // init pins
    pinMode(PIN_STATUSLED, OUTPUT);
    pinMode(PIN_LOWER_SENSOR, INPUT_PULLUP);
    pinMode(PIN_UPPER_SENSOR, INPUT_PULLUP);

    Serial.begin(9600);
}

// Interrupts
void lowerSensorISR() {
    mode.lastImpulseTimes.lower_sensor = millis();
}
void upperSensorISR() {
    mode.lastImpulseTimes.upper_sensor = millis();
}

// Main
void setup() {
    init();
    
    log(Log::LOG, F("Setup"));
    mode.initialize();

    log(Log::LOG, F("Setting up interrupts"));
    attachInterrupt(digitalPinToInterrupt(PIN_LOWER_SENSOR), lowerSensorISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_UPPER_SENSOR), upperSensorISR, FALLING);
    
    log(Log::LOG, F("Ready"));
}

void loop() {
    mode.loop();
}
