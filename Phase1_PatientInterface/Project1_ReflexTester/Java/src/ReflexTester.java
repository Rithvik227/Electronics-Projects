/*
 * =====================================================================================
 * Project 1: Hardware-Debounced Neurological Reflex Tester (NRT-1) - Java Software Twin
 * Phase 1 - Summer 2026 Engineering Master Plan
 * * Author: Rithvik
 * Date: June 2026
 * * Description:
 * A Java-based Object-Oriented software twin of the bare-metal C++ NRT-1. Simulates
 * embedded microcontroller hardware timings using System.nanoTime(). Replicates the
 * exact non-blocking FSM logic and implements a synthetic terminal input bridge to
 * mimic physical circuit bouncing and line-buffered OS latency.
 * * Virtual Hardware Mapping:
 * - Start Button  (Green) : Keyboard 's' + Enter (Simulates Digital Pin 2)
 * - Reflex Button (Red)   : Keyboard 'r' + Enter (Simulates Digital Pin 3)
 * - RGB LED Channels      : Standard Console Output (Simulates Pins 9, 10, 11)
 * * Final Commit Updates:
 * - Resolved all strict Java linting warnings (dead code, busy-waiting, infinite loops).
 * - Built a synthetic 100ms hold timer to pass OS terminal streams through the debounce filter.
 * - Synchronized FSM interval bounds and anti-cheat trapdoors with the C++ architecture.
 * - Some comments and code have been written with the Assistance of Gemini 3.1 Pro.
 * =====================================================================================
 */

import java.io.IOException;
import java.util.Random;

interface PatientInterface {
    String getComponentName();
}

public class ReflexTester {

    // Helper method to simulate Arduino millis()
    private static long getTimeNano() {
        return (System.nanoTime() / 1000000);
    }

    // Enumeration for State Machine
    enum SystemState {
        IDLE,
        PRE_TEST_BLINK,
        AWAIT_REACTION,
        EVALUATE_RESULTS
    }

    static class DebouncedButton implements PatientInterface {
        private final int pin;
        private final char keyCharacter;
        private final long debounceThreshold;

        private boolean currentButtonState;
        private boolean lastStableState;
        private boolean lastRawState;
        private boolean previousLoopState;
        private long lastTransitionTime;

        // Java Terminal hold due to simulated "button presses"
        private long syntheticHoldUntil = 0;

        public DebouncedButton(int pin, char keyCharacter, long debounceThreshold) {
            this.pin = pin;
            this.keyCharacter = keyCharacter;
            this.debounceThreshold = debounceThreshold;

            if (keyCharacter == 's') {
                System.out.println("The Start button has been assigned to pin " + this.pin);
            } else if (keyCharacter == 'r') {
                System.out.println("The Reflex button has been assigned to pin " + this.pin);
            }
        }

        public boolean isCurrentlyPressed() {
            //From Gemini AI: Made a way to handle the terminal input without using blocking loops
            try {
                // Read all available characters in the buffer to empty it properly
                while (System.in.available() > 0) {
                    int inputChar = System.in.read();
                    if (inputChar == this.keyCharacter) {
                        // Simulate a physical human finger holding the button for 100ms
                        this.syntheticHoldUntil = getTimeNano() + 100;
                    }
                }
            } catch (IOException e) {
                System.err.println("Error reading terminal input.");
            }

            // The pin acts as "HIGH" as long as the synthetic timer hasn't expired
            this.currentButtonState = (getTimeNano() < this.syntheticHoldUntil);

            if (this.currentButtonState != this.lastRawState) {
                this.lastTransitionTime = getTimeNano();
            }

            if (getTimeNano() - this.lastTransitionTime > this.debounceThreshold) {
                this.lastStableState = this.currentButtonState;
            }

            this.lastRawState = this.currentButtonState;
            return this.lastStableState;
        }

        public boolean hasChangedState() {
            boolean currentState = this.isCurrentlyPressed();
            boolean wasJustPressed = false;

            if (currentState != this.previousLoopState) {
                if (currentState) {
                    this.lastStableState = currentState;
                    wasJustPressed = true;
                }
            }

            this.previousLoopState = currentState;
            return wasJustPressed;
        }

        @Override
        public String getComponentName() {
            return "Pushbutton Unit";
        }
    }

    static class RGBLed implements PatientInterface {
        private int rState;
        private int gState;
        private int bState;

        public RGBLed(int r, int g, int b) {
            System.out.println("The RGB led's pins are assigned to pins : " + r + ", " + g + ", " + b);

            this.rState = -1;
            this.gState = -1;
            this.bState = -1;
        }

        public void writeColor(int r, int g, int b) {
            if (this.rState != r || this.gState != g || this.bState != b) {
                if (r > 0) {
                    System.out.println("RED LED PIN SET TO HIGH");
                } else {
                    System.out.println("RED LED PIN SET TO LOW");
                }

                if (g > 0) {
                    System.out.println("GREEN LED PIN SET TO HIGH");
                } else {
                    System.out.println("GREEN LED PIN SET TO LOW");
                }

                if (b > 0) {
                    System.out.println("BLUE LED PIN SET TO HIGH");
                } else {
                    System.out.println("BLUE LED PIN SET TO LOW");
                }

                System.out.println();
                this.rState = r;
                this.gState = g;
                this.bState = b;
            }
        }

        public void clear() {
            this.writeColor(0, 0, 0);
        }

        @Override
        public String getComponentName() {
            return "RGB LED Indicator Unit";
        }
    }

    static void main(String[] args) {
        DebouncedButton startButton = new DebouncedButton(2, 's', 50);
        DebouncedButton reflexButton = new DebouncedButton(3, 'r', 50);
        RGBLed led = new RGBLed(9, 10, 11);
        Random rand = new Random();

        SystemState currentState = SystemState.IDLE;
        long blinkStart = 0;
        long reactionTestStartTime = 0;
        long reactionTestFinishTime = 0;
        long blinkPeriod = 0;
        int blinkCount = 0;
        int targetBlinks = 0;
        boolean blinkState = false;

        led.clear();

        while (true) {
            long currentTicks = getTimeNano();

            switch (currentState) {
                case IDLE:
                    led.clear();

                    if (startButton.hasChangedState()) {
                        currentState = SystemState.PRE_TEST_BLINK;
                        long randomBlinkInterval = 3000 + rand.nextInt(3001); // Maps to the 3000-6000 bounds
                        targetBlinks = 3 + rand.nextInt(5);
                        blinkPeriod = randomBlinkInterval / targetBlinks;
                        System.out.println("Reaction time test is about to begin. Test will begin very shortly. Please be alert. Press the red button as soon you react to the LED changing color.");

                        System.out.println("MTRX," + currentTicks + ",PRE_TEST_BLINK,0");

                        blinkStart = currentTicks;
                        blinkState = true;
                    }
                    break;

                case PRE_TEST_BLINK:
                    if (reflexButton.hasChangedState()) {
                        currentState = SystemState.IDLE;
                        blinkPeriod = 0;
                        blinkCount = 0;
                        targetBlinks = 0;
                        blinkState = false;
                        led.clear();
                        System.out.println("Premature button press detected. Input will be classified as cheating. Resetting test. Please try again and do not press the red button until the LED turns green");
                        System.out.println("MTRX," + currentTicks + ",IDLE,CHEAT");
                        break;
                    }

                    if (blinkCount != targetBlinks) {
                        if (blinkState) {
                            led.writeColor(255,0,0);
                        } else {
                            led.clear();
                        }

                        if (currentTicks - blinkStart >= blinkPeriod) {
                            blinkState = !blinkState;
                            blinkStart = currentTicks;
                            blinkCount++;
                        }
                    } else {
                        blinkCount = 0;
                        led.clear();
                        System.out.println("Green light has been activated. The Reaction Time Test has begun. Press the red button.");
                        led.writeColor(0,255,0);
                        reactionTestStartTime = getTimeNano();
                        currentState = SystemState.AWAIT_REACTION;

                        System.out.println("MTRX," + reactionTestStartTime + ",AWAIT_REACTION,0");
                    }
                    break;

                case AWAIT_REACTION:
                    if (reflexButton.hasChangedState()) {
                        //Subtracted the debounce delay from the finish time b/c the button press is detected after the forced debounce delay
                        reactionTestFinishTime = getTimeNano() - 50;
                        currentState = SystemState.EVALUATE_RESULTS;
                    }
                    break;

                case EVALUATE_RESULTS:
                    led.clear();

                    long totalTicks = reactionTestFinishTime - reactionTestStartTime;

                    System.out.println("The Reaction Time test has concluded.");
                    System.out.println("Your reaction time was: " + totalTicks + " ms.");
                    System.out.println("Press the green button again anytime to try again.");

                    System.out.println("MTRX," + reactionTestFinishTime + ",EVALUATE_RESULTS," + totalTicks);

                    currentState = SystemState.IDLE;
                    break;
            }

            // From Gemini AI: Prevents the OS from freezing the terminal input thread
            try { Thread.sleep(1); } catch (InterruptedException ignored) {}
        }
    }
}