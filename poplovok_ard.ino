#include "poplovok_ard.h"


// State
enum State {
    ST_SETUP,
    ST_PUMPING,
    ST_IDLING
};

State globalState = ST_SETUP;

// Modified by interrupts
volatile SensorUpdateTime lastImpulseTime{0,0};
volatile SensorUpdateTime lastHandledImpulseTime{0,0};
volatile bool upperSensorFlag = false;
volatile bool lowerSensorFlag = false;

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
    const unsigned long now = millis();
    // Raise flag unless already raised recently.
    if (now - lastHandledImpulseTime.lower_sensor > SENSOR_DEBOUNCING_TIME) 
    {
        lowerSensorFlag = true;
        lastHandledImpulseTime.lower_sensor = now;
    }
    // Always update last impulse time
    lastImpulseTime.lower_sensor = now;
}
void upperSensorISR() {
    const unsigned long now = millis();
    // Raise flag unless already raised recently.
    if (now - lastHandledImpulseTime.upper_sensor > SENSOR_DEBOUNCING_TIME) 
    {
        upperSensorFlag = true;
        lastHandledImpulseTime.upper_sensor = now;
    }
    // Always update last impulse time
    lastImpulseTime.upper_sensor = now;
}

// Main
void setup() {
    setup_inputs();
    
    info(F("Hello"));

    info(F("Setting up interrupts"));
    // FALLING because inputs configured as input_pullup, default state is HIGH, and impulse is LOW.
    attachInterrupt(digitalPinToInterrupt(PIN_LOWER_SENSOR), lowerSensorISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_UPPER_SENSOR), upperSensorISR, FALLING);
    
    info(F("Ready"));
    globalState = ST_IDLING;
}

void applyState(State newState) {
    if (globalState == ST_PUMPING && newState == ST_IDLING) {
        info(F("ST_PUMPING -> ST_IDLING"));
        info(F("stop pump"));
        digitalWrite(PIN_PUMP, PUMP_OFF);
        digitalWrite(PIN_STATUSLED, LOW);
    }
    if (globalState == ST_IDLING && newState == ST_PUMPING) {
        info(F("ST_IDLING -> ST_PUMPING"));
        info(F("run pump"));
        digitalWrite(PIN_PUMP, PUMP_ON);
        digitalWrite(PIN_STATUSLED, HIGH);
    }

    globalState = newState;
}

void loop() {
    char lowerVal = digitalRead(PIN_LOWER_SENSOR) == HIGH ? '1' : '0';
    char upperVal = digitalRead(PIN_UPPER_SENSOR) == HIGH ? '1' : '0';
    verbose(((String(F("l,u = ")) + lowerVal) + ',') + upperVal);

    if (globalState == ST_IDLING) 
    {
        if (upperSensorFlag) 
        {
            info(String(F("Handling upper impulse. t")) + (String(lastImpulseTime.upper_sensor, DEC) + F(" h")) + String(lastHandledImpulseTime.upper_sensor, DEC));
            applyState(ST_PUMPING);
            upperSensorFlag = false;
            lowerSensorFlag = false;
        }
    } 
    else
    if (globalState == ST_PUMPING) 
    {
        if (lowerSensorFlag)
        {
            info(String(F("Handling lower impulse. t")) + (String(lastImpulseTime.lower_sensor, DEC) + F(" h")) + String(lastHandledImpulseTime.lower_sensor, DEC));
            applyState(ST_IDLING);
            upperSensorFlag = false;
            lowerSensorFlag = false;
        }
    }

}
