//Reaction time tester
#include <Arduino.h>

class DebouncedButton {
private:
    int _pin;
    bool _lastStableState;
    bool _lastRawState;
    bool _previousLoopState;
    unsigned long _lastTransitionTime;
    unsigned long _debounceThreshold;

public:
    DebouncedButton(int pin, unsigned long debounceThreshold) : _pin(pin), _debounceThreshold(debounceThreshold) {}

    void init() {
        pinMode(_pin, INPUT);
        _lastStableState = LOW;
        _lastRawState = LOW;
        _previousLoopState = LOW;
        _lastTransitionTime = 0;
    }

    bool isCurrentlyPressed() {
        bool state = digitalRead(_pin);

        if (state != _lastRawState) {
            _lastTransitionTime = millis();
        }

        if (millis() - _lastTransitionTime >= _debounceThreshold) {
            _lastStableState = state;
        }

        _lastRawState = state;

        return _lastStableState;
    }

    bool hasChangedState() {
        bool currentStable = isCurrentlyPressed();

        bool wasJustPressed = false;

        if (currentStable != _previousLoopState) {
            if (currentStable == HIGH) {
                wasJustPressed = true;
            }
        }
        _previousLoopState = currentStable;
        return wasJustPressed;
    }
};

class RGBLed {
private:
    int _rPin, _gPin, _bPin;

public:
    RGBLed(int r, int g, int b) : _rPin(r), _gPin(g), _bPin(b) {}

    void init() {
        pinMode(_rPin, OUTPUT);
        pinMode(_gPin, OUTPUT);
        pinMode(_bPin, OUTPUT);
    }

    void writeColor(int r, int g, int b) {
        //AI Suggestion for future: digitalWrite(_rPin, r > 0 ? HIGH : LOW);
        if (r > 0) {
            digitalWrite(_rPin, HIGH);
        } else {
            digitalWrite(_rPin, LOW);
        }

        if (g > 0) {
            digitalWrite(_gPin, HIGH);
        } else {
            digitalWrite(_gPin, LOW);
        }

        if (b > 0) {
            digitalWrite(_bPin, HIGH);
        } else {
            digitalWrite(_bPin, LOW);
        }
    }

    void clear() {
        writeColor(0,0,0);
    }
};

enum SystemState {
    IDLE,
    PRE_TEST_BLINK,
    AWAIT_REACTION,
    EVALUATE_RESULTS
};

DebouncedButton startButton(2, 50);
DebouncedButton reflexButton(3, 50);
RGBLed LED(9,10,11);

SystemState currentState = IDLE;
unsigned long stateTimer = 0;
unsigned long blinkStart = 0;
unsigned long reactionTestStartTime = 0;
unsigned long reactionTestFinishTime = 0;
long randomBlinkInterval = 0;
long blinkPeriod = 0;
int blinkCount = 0;
int targetBlinks = 0;
bool blinkState = false;

void setup() {
    startButton.init();
    reflexButton.init();
    LED.init();
    randomSeed(analogRead(A0));
    Serial.begin(115200);
}

void loop() {
    switch (currentState) {
        case IDLE:
            LED.clear();

            if (startButton.hasChangedState()) {
                currentState = PRE_TEST_BLINK;
                randomBlinkInterval = random(5000, 8000);
                targetBlinks = random(3, 7);
                blinkPeriod = randomBlinkInterval / targetBlinks;
                Serial.println("Reaction time test is about to begin. Test will begin very shortly. Please be alert. Press the red button as soon you react to the LED changing color.");
                blinkStart = millis();

                blinkState = true;
            }
            break;

        case PRE_TEST_BLINK:
            if (blinkCount != targetBlinks) {
                if (blinkState == true) {
                    LED.writeColor(255,0,0);
                } else {
                    LED.clear();
                }

                if (millis() - blinkStart >= blinkPeriod) {
                    blinkState = !blinkState;
                    blinkStart = millis();
                    blinkCount++;
                }
            } else {
                randomBlinkInterval = 0;
                blinkCount = 0;
                blinkState = false;
                LED.clear();
                Serial.println("The Reaction Time Test has begun. Press the red button.");
                LED.writeColor(0,255,0);
                reactionTestStartTime = millis();
                currentState = AWAIT_REACTION;
            }
            break;

        case AWAIT_REACTION:
            if (reflexButton.hasChangedState()) {
                //Subtracted the debounce delay from the finish time b/c the button press is detected after the forced debounce delay
                reactionTestFinishTime = millis() - 50;
                currentState = EVALUATE_RESULTS;
            }
            break;

        case EVALUATE_RESULTS:
            Serial.println("The Reaction Time test has concluded.");
            String testResult = String("Your reaction time was ") + (reactionTestFinishTime - reactionTestStartTime) + " ms.";
            Serial.println(testResult);
            Serial.println("Press the green button again anytime to try again.");
            currentState = IDLE;
            break;
    }
}

