# Python Software Twin of the C++ Script for Project 1 for testing using OOP
# Followed an example of a patient call button for first time doing python OOP.
# Skipping the software twin of the real original C++ code and going straight to new C++ OOP version's software twin.
from abc import ABC, abstractmethod
from enum import Enum
import time
import keyboard
import random

def get_ticks():
    # Convert PC nanoseconds into Arduino-scaled ticks
    return int(time.perf_counter_ns() / 1000000)

class PatientInterface(ABC):
    @abstractmethod
    def get_component_name(self):
        pass

class DebouncedButton(PatientInterface):

    def __init__(self, pin, key_character, debounceThreshold):
        print(f"The button is assigned to pin: {pin}")
        print(f"The button's debounce delay is set to: {debounceThreshold}")
        self._pin = pin
        self._lastTransitionTime = 0
        self._key_character = key_character
        self._debounceThreshold = debounceThreshold
        self._currentButtonState = False
        self._lastStableState = False
        self._lastRawState = False
        self._previousLoopState = False

    def isCurrentlyPressed(self):
        self._currentButtonState = keyboard.is_pressed(self._key_character)

        if (self._currentButtonState != self._lastRawState):
            self._lastTransitionTime = get_ticks()

        if (get_ticks() - self._lastTransitionTime >= self._debounceThreshold):
            self._lastStableState = self._currentButtonState

        self._lastRawState = self._currentButtonState

        return self._lastStableState

    def hasChangedState(self):
        currentStable = self.isCurrentlyPressed()

        wasJustPressed = False

        if (currentStable != self._previousLoopState):
            if (currentStable == True):
                wasJustPressed = True

        self._previousLoopState = currentStable
        return wasJustPressed

    def get_component_name(self):
        return "Pushbutton"

class RGBLed(PatientInterface):
    def __init__(self, r, g, b):
        self._rPin = r
        self._gPin = g
        self._bPin = b
        self._rState = -1
        self._gState = -1
        self._bState = -1
        print(f"The RGB led's pins are assigned to pins : {self._rPin}, {self._gPin}, {self._bPin} ")

    def writeColor(self, r,  g,  b):
        if (r != self._rState or g != self._gState or b != self._bState):
            if (r > 0):
                print("RED LED PIN SET TO HIGH")
            else:
                print("RED LED PIN SET TO LOW")

            if (g > 0):
                print("GREEN LED PIN SET TO HIGH")
            else:
                print("GREEN LED PIN SET TO LOW")

            if (b > 0):
                print("BLUE LED PIN SET TO HIGH")
            else:
                print("BLUE LED PIN SET TO LOW")

            print()

            self._rState = r
            self._gState = g
            self._bState = b

    def clear(self):
        self.writeColor(0,0,0)

    def get_component_name(self):
        return "RGB LED Indicator"

class SystemState(Enum):
    IDLE = 1
    PRE_TEST_BLINK = 2
    AWAIT_REACTION = 3
    EVALUATE_RESULTS = 4

startButton = DebouncedButton(2, 's',50)
reflexButton = DebouncedButton(3, 'r',50)
LED = RGBLed(9, 10, 11)

# Initialize System Variables
currentState = SystemState.IDLE
stateTimer = 0
blinkStart = 0
reactionTestStartTime = 0
reactionTestFinishTime = 0
randomBlinkInterval = 0
blinkPeriod = 0
blinkCount = 0
targetBlinks = 0
blinkState = False

if __name__ == "__main__":
    while True:
        match currentState:
            case SystemState.IDLE:
                LED.clear()

                if (startButton.hasChangedState()):
                    currentState = SystemState.PRE_TEST_BLINK
                    randomBlinkInterval = random.randint(5000, 8000)
                    targetBlinks = random.randint(3,7)
                    blinkPeriod = randomBlinkInterval / targetBlinks
                    print("Reaction time test is about to begin. Test will begin very shortly. Please be alert. Press the red button as soon you react to the LED changing color.")
                    blinkStart = get_ticks()
                    blinkState = True

            case SystemState.PRE_TEST_BLINK:
                if (blinkCount != targetBlinks):
                    if (blinkState == True):
                        LED.writeColor(255,0,0)
                    else:
                        LED.clear()

                    if (get_ticks() - blinkStart >= blinkPeriod):
                        blinkStart = get_ticks()
                        blinkState = not blinkState
                        blinkCount += 1
                else:
                    randomBlinkInterval = 0
                    blinkPeriod = 0
                    blinkCount = 0
                    targetBlinks = 0
                    blinkState = False
                    LED.clear()
                    print("The Reaction Time Test has begun. Press the red button.")
                    LED.writeColor(0,255,0)
                    reactionTestStartTime = get_ticks()
                    currentState = SystemState.AWAIT_REACTION

            case SystemState.AWAIT_REACTION:
                    if (reflexButton.hasChangedState()):
                        # Subtracted the debounce delay from the finish time b/c the button press is detected after the forced debounce delay
                        reactionTestFinishTime = get_ticks() - 50
                        currentState = SystemState.EVALUATE_RESULTS

            case SystemState.EVALUATE_RESULTS:
                print("The Reaction Time test has concluded.")
                print(f"Your reaction time was {reactionTestFinishTime - reactionTestStartTime} ms.")
                print("Press the green button again anytime to try again.")
                currentState = SystemState.IDLE






