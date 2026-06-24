"""
=====================================================================================
Project 1: Hardware-Debounced Neurological Reflex Tester (NRT-1) - Python Software Twin
Phase 1 - Summer 2026 Engineering Master Plan
* Author: Rithvik
Date: June 2026
* Description:
A Python-based Object-Oriented software twin of the bare-metal C++ NRT-1. Utilizes
time.perf_counter_ns() for hardware-level microsecond resolution and OS-level USB
keyboard hooks to perfectly mimic physical GPIO state changes. Completely bypasses
blocking OS timers to match the exact embedded C++ Finite State Machine architecture.
* Virtual Hardware Mapping:
- Start Button  (Green) : Keyboard 's' key (Simulates Digital Pin 2)
- Reflex Button (Red)   : Keyboard 'r' key (Simulates Digital Pin 3)
- RGB LED Channels      : Standard Console Output (Simulates Pins 9, 10, 11)
* Final Commit Updates:
- Enforced strict PEP 8 compliance (snake_case translation, formatting).
- Integrated asynchronous hardware edge-detection logic via the 'keyboard' module.
- Some comments and code have been written with the Assistance of Gemini 3.1 Pro.
=====================================================================================
"""

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

    def __init__(self, pin, key_character, debounce_threshold):
        print(f"The button is assigned to pin: {pin}")
        print(f"The button's debounce delay is set to: {debounce_threshold}")
        self._pin = pin
        self._last_transition_time = 0
        self._key_character = key_character
        self._debounce_threshold = debounce_threshold
        self._current_button_state = False
        self._last_stable_state = False
        self._last_raw_state = False
        self._previous_loop_state = False

    def is_currently_pressed(self):
        self._current_button_state = keyboard.is_pressed(self._key_character)

        if self._current_button_state != self._last_raw_state:
            self._last_transition_time = get_ticks()

        if get_ticks() - self._last_transition_time >= self._debounce_threshold:
            self._last_stable_state = self._current_button_state

        self._last_raw_state = self._current_button_state
        return self._last_stable_state

    def has_changed_state(self):
        current_stable = self.is_currently_pressed()
        was_just_pressed = False

        if current_stable != self._previous_loop_state:
            if current_stable:
                was_just_pressed = True

        self._previous_loop_state = current_stable
        return was_just_pressed

    def get_component_name(self):
        return "Pushbutton"


class RGBLed(PatientInterface):
    def __init__(self, r, g, b):
        self._r_pin = r
        self._g_pin = g
        self._b_pin = b
        self._r_state = -1
        self._g_state = -1
        self._b_state = -1
        print(f"The RGB led's pins are assigned to pins : {self._r_pin}, {self._g_pin}, {self._b_pin} ")

    def write_color(self, r, g, b):
        if r != self._r_state or g != self._g_state or b != self._b_state:
            if r > 0:
                print("RED LED PIN SET TO HIGH")
            else:
                print("RED LED PIN SET TO LOW")

            if g > 0:
                print("GREEN LED PIN SET TO HIGH")
            else:
                print("GREEN LED PIN SET TO LOW")

            if b > 0:
                print("BLUE LED PIN SET TO HIGH")
            else:
                print("BLUE LED PIN SET TO LOW")

            print()

            self._r_state = r
            self._g_state = g
            self._b_state = b

    def clear(self):
        self.write_color(0, 0, 0)

    def get_component_name(self):
        return "RGB LED Indicator"


class SystemState(Enum):
    IDLE = 1
    PRE_TEST_BLINK = 2
    AWAIT_REACTION = 3
    EVALUATE_RESULTS = 4


start_button = DebouncedButton(2, 's', 50)
reflex_button = DebouncedButton(3, 'r', 50)
LED = RGBLed(9, 10, 11)

# Initialize System Variables
current_state = SystemState.IDLE
state_timer = 0
blink_start = 0
reaction_test_start_time = 0
reaction_test_finish_time = 0
random_blink_interval = 0
blink_period = 0
blink_count = 0
target_blinks = 0
blink_state = False

if __name__ == "__main__":
    while True:
        match current_state:
            case SystemState.IDLE:
                LED.clear()

                if start_button.has_changed_state():
                    current_state = SystemState.PRE_TEST_BLINK
                    random_blink_interval = random.randint(3000, 6000)
                    target_blinks = random.randint(3, 7)
                    blink_period = random_blink_interval / target_blinks
                    print(
                        "Reaction time test is about to begin. Test will begin very shortly. Please be alert. Press the red button as soon you react to the LED changing color.")

                    print(f"MTRX,{get_ticks()},PRE_TEST_BLINK,0")

                    blink_start = get_ticks()
                    blink_state = True

            case SystemState.PRE_TEST_BLINK:
                if reflex_button.has_changed_state():
                    current_state = SystemState.IDLE
                    blink_period = 0
                    blink_count = 0
                    target_blinks = 0
                    blink_state = False
                    LED.clear()
                    print(
                        "Premature button press detected. Input will be classified as cheating. Resetting test. Please try again and do not press the red button until the LED turns green")
                    print(f"MTRX,{get_ticks()},IDLE,CHEAT")

                elif blink_count != target_blinks:
                    if blink_state:
                        LED.write_color(255, 0, 0)
                    else:
                        LED.clear()

                    if get_ticks() - blink_start >= blink_period:
                        blink_start = get_ticks()
                        blink_state = not blink_state
                        blink_count += 1
                else:
                    blink_count = 0
                    LED.clear()
                    print("Green light has been activated. The Reaction Time Test has begun. Press the red button.")
                    LED.write_color(0, 255, 0)
                    reaction_test_start_time = get_ticks()
                    current_state = SystemState.AWAIT_REACTION

                    print(f"MTRX,{reaction_test_start_time},AWAIT_REACTION,0")

            case SystemState.AWAIT_REACTION:
                if reflex_button.has_changed_state():
                    # Subtracted the debounce delay from the finish time b/c the button press is detected after the forced debounce delay
                    reaction_test_finish_time = get_ticks() - 50
                    current_state = SystemState.EVALUATE_RESULTS

            case SystemState.EVALUATE_RESULTS:
                LED.clear()
                print("The Reaction Time test has concluded.")
                print(f"Your reaction time was: {reaction_test_finish_time - reaction_test_start_time} ms.")
                print("Press the green button again anytime to try again.")

                print(
                    f"MTRX,{reaction_test_finish_time},EVALUATE_RESULTS,{reaction_test_finish_time - reaction_test_start_time}")

                current_state = SystemState.IDLE