/*
* =====================================================================================
 * Project 1: Hardware-Debounced Neurological Reflex Tester (NRT-1)
 * Phase 1 - Summer 2026 Engineering Master Plan
 * * Author: Rithvik
 * Date: June 2026 
 * * Description:
 * A bare-metal, heap-free C++ implementation of a clinical-grade reaction
 * time tester. Utilizes the ATmega328P 16-bit Hardware Timer 1 (1024 prescaler)
 * for deterministic precision (±64 µs accuracy). Architecture features custom
 * non-blocking OOP debouncers, a strictly defined Finite State Machine (FSM),
 * and explicit 32-bit integer promotion to prevent temporal math overflow.
 * * Hardware Pin Mapping (ATmega328P):
 * - Start Button  (Green) : Digital Pin 2  (PORTD, Bit 2)
 * - Reflex Button (Red)   : Digital Pin 3  (PORTD, Bit 3)
 * - RGB LED Red   Channel : Digital Pin 9  (PORTB, Bit 1)
 * - RGB LED Green Channel : Digital Pin 10 (PORTB, Bit 2)
 * - RGB LED Blue  Channel : Digital Pin 11 (PORTB, Bit 3)
 * * Final Commit Updates:
 * - Resolved all Clang-Tidy static analysis warnings (narrowing conversions, const-correctness).
 * - Eliminated dynamic heap allocation (String class) to guarantee 0% memory fragmentation.
 * - Implemented continuous hardware edge-detection and FSM anti-cheat trapdoors.
 * - Some comments and code have been written with the Assistance of Gemini 3.1 Pro
 * =====================================================================================
 */
#include <Arduino.h>

class DebouncedButton {
private:
    uint8_t _bitPos;
    bool _lastStableState;
    bool _lastRawState;
    bool _previousLoopState;
    uint16_t _lastTransitionTime;
    uint16_t _debounceThresholdTicks;

public:
    DebouncedButton(uint8_t bitPos, uint16_t debounceThresholdTicks)
        : _bitPos(bitPos),
          _lastStableState(false),
          _lastRawState(false),
          _previousLoopState(false),
          _lastTransitionTime(0),
          _debounceThresholdTicks(debounceThresholdTicks) {}

    void init() {
        DDRD &= ~(1 << _bitPos);
        _lastStableState = false;
        _lastRawState = false;
        _previousLoopState = false;
        _lastTransitionTime = 0;
    }

    bool isCurrentlyPressed() {
        bool state = (PIND & (1 << _bitPos)) ? true : false;

        uint16_t currentTicks = TCNT1;

        if (state != _lastRawState) {
            _lastTransitionTime = currentTicks;
        }

        if (static_cast<uint16_t>(currentTicks - _lastTransitionTime) >= _debounceThresholdTicks) {
            _lastStableState = state;
        }

        _lastRawState = state;
        return _lastStableState;
    }

    bool hasChangedState() {
        bool currentStable = isCurrentlyPressed();

        bool wasJustPressed = false;

        if (currentStable != _previousLoopState) {
            if (currentStable == true) {
                wasJustPressed = true;
            }
        }
        _previousLoopState = currentStable;
        return wasJustPressed;
    }
};

class RGBLed {
private:
    uint8_t _rBit, _gBit, _bBit;

public:
    RGBLed(uint8_t rBit, uint8_t gBit, uint8_t bBit) : _rBit(rBit), _gBit(gBit), _bBit(bBit) {}

    void init() const {
        DDRB |= (1 << _rBit) | (1 << _gBit) | (1 << _bBit);
    }

    void writeColor(uint8_t r, uint8_t g, uint8_t b) const {
        //AI Suggestion for future: digitalWrite(_rPin, r > 0 ? HIGH : LOW);
        if (r > 0) {
            PORTB |= (1 << _rBit);
        } else {
            PORTB &= ~(1 << _rBit);
        }

        if (g > 0) {
            PORTB |= (1 << _gBit);
        } else {
            PORTB &= ~(1 << _gBit);
        }

        if (b > 0) {
            PORTB |= (1 << _bBit);
        } else {
            PORTB &= ~(1 << _bBit);
        }
    }

    void clear() const {
        writeColor(0,0,0);
    }
};

enum SystemState {
    IDLE,
    PRE_TEST_BLINK,
    AWAIT_REACTION,
    EVALUATE_RESULTS
};

DebouncedButton startButton(2, 781);
DebouncedButton reflexButton(3, 781);
RGBLed LED(1,2,3);

SystemState currentState = IDLE;

uint16_t blinkStart = 0;
uint16_t reactionTestStartTime = 0;
uint16_t reactionTestFinishTime = 0;
uint16_t clockTicksPerMillisecond = 16;

uint32_t randomBlinkInterval = 0;
uint32_t blinkPeriod = 0;

long blinkCount = 0;
long targetBlinks = 0;
bool blinkState = false;

void setup() {
    startButton.init();
    reflexButton.init();
    LED.init();

    TCCR1A = 0; //Clear Control Register A
    TCCR1B = 0; //Clear Control Register B
    TCCR1B |= (1 << CS12) | (1 << CS10); // Set prescaler to 1024

    //Seed using the raw analog input from register 0
    randomSeed(analogRead(A0));
    Serial.begin(115200);
}

void loop() {
    uint16_t currentTicks = TCNT1;

    switch (currentState) {
        case IDLE:
            LED.clear();

            if (startButton.hasChangedState()) {
                currentState = PRE_TEST_BLINK;

                //Converts random millisecond requirements into Timer 1 ticks
                auto rawIntervalMs = random(3000, 6000); // 3 to 6 seconds now perfectly allowed
                randomBlinkInterval = static_cast<uint32_t>(static_cast<double>(rawIntervalMs) * 15.625);
                targetBlinks = random(3, 7);
                blinkPeriod = randomBlinkInterval / static_cast<uint32_t>(targetBlinks);
                Serial.println("Reaction time test is about to begin. Test will begin very shortly. Please be alert. Press the red button as soon you react to the LED changing color.");

                Serial.print("MTRX,"); Serial.print(currentTicks); Serial.println(",PRE_TEST_BLINK,0");

                blinkStart = currentTicks;
                blinkState = true;
            }
            break;

        case PRE_TEST_BLINK:
            if (reflexButton.hasChangedState()) {
                currentState = IDLE;
                blinkPeriod = 0;
                blinkCount = 0;
                targetBlinks = 0;
                blinkState = false;
                LED.clear();
                Serial.println("Premature button press detected. Input will be classified as cheating. Resetting test. Please try again and do not press the red button until the LED turns green");
                Serial.print("MTRX,"); Serial.print(currentTicks); Serial.println(",IDLE,CHEAT");
                break;
            }

            if (blinkCount != targetBlinks) {
                if (blinkState == true) {
                    LED.writeColor(255,0,0);
                } else {
                    LED.clear();
                }

                if (static_cast<uint32_t>(static_cast<uint16_t>(currentTicks - blinkStart)) >= blinkPeriod) {
                    blinkState = !blinkState;
                    blinkStart = currentTicks;
                    blinkCount++;
                }
            } else {
                blinkCount = 0;
                LED.clear();
                Serial.println("Green light has been activated. The Reaction Time Test has begun. Press the red button.");
                LED.writeColor(0,255,0);
                reactionTestStartTime = TCNT1;
                currentState = AWAIT_REACTION;

                Serial.print("MTRX,"); Serial.print(reactionTestStartTime); Serial.println(",AWAIT_REACTION,0");
            }
            break;

        case AWAIT_REACTION:
            if (reflexButton.hasChangedState()) {
                //Subtracted the debounce delay from the finish time b/c the button press is detected after the forced debounce delay
                reactionTestFinishTime = static_cast<uint16_t>(TCNT1 - 781);
                currentState = EVALUATE_RESULTS;
            }
            break;

        case EVALUATE_RESULTS:
            LED.clear();

            //Get the raw ticks (this fits safely in 16-bit)
            auto totalTicks = static_cast<uint16_t>(reactionTestFinishTime - reactionTestStartTime);

            //Promote to 32-bit first, multiply by 1000UL, divide, then cast back down to 16-bit
            auto totalMilliseconds = static_cast<uint16_t>((static_cast<uint32_t>(totalTicks) * 1000UL) / 15625UL);

            Serial.println("The Reaction Time test has concluded.");
            Serial.println("Your reaction time was: ");
            Serial.print(totalMilliseconds);
            Serial.print(" ms.");
            Serial.println("Press the green button again anytime to try again.");

            Serial.print("MTRX,");
            Serial.print(reactionTestFinishTime);
            Serial.print(",EVALUATE_RESULTS,");
            Serial.println(totalMilliseconds);

            currentState = IDLE;
            break;
    }
}

