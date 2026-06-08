//Reflex tester
#include <Arduino.h>

const int buttonOnePin = 2;
const int buttonTwoPin = 3;
const int debounceDelay = 50;
int redPin = 9;
int greenPin = 10;
int bluePin = 11;
bool testActive = false;
unsigned long buttonOneLastPressed = 0;
int lastButtonOneState = LOW;

void setColor(int redValue, int greenValue, int blueValue);

void changeLedStatus (int red, int green, int blue) {
    analogWrite(redPin, red);
    analogWrite(greenPin, green);
    analogWrite(bluePin, blue);
}

void setup() {
    pinMode(buttonOnePin,INPUT);
    pinMode(buttonTwoPin,INPUT);
    pinMode(redPin,OUTPUT);
    pinMode(greenPin,OUTPUT);
    pinMode(bluePin,OUTPUT);
    changeLedStatus(0,0,0);
    Serial.begin(115200);
}

long getRandomInt(int min,int max) {
    randomSeed(analogRead(A0));
    return random(min, max + 1);
}

void loop() {
    int currentButtonOneState = digitalRead(buttonOnePin);

    if (currentButtonOneState != lastButtonOneState) {
        if (millis() - buttonOneLastPressed >= debounceDelay) {
            if (testActive != true) {
                if (currentButtonOneState == HIGH) {
                    testActive = true;
                    Serial.println("Reaction time test is about to begin. Test will begin very shortly. Please be alert. Press the red button as soon you react to the LED changing color.");
                    long randomInt = getRandomInt(7000,9000);

                    for (long i = 0; i < getRandomInt(5, 10); i++) {
                        changeLedStatus(255,0,0);
                        delay(randomInt / 12);
                        changeLedStatus(0,0,0);
                        delay(randomInt / 12);
                    }

                    unsigned long ledOnTime = millis();
                    changeLedStatus(0,255,0);
                    Serial.println("The Reaction Time Test has begun. Press the red button.");

                    while (digitalRead(buttonTwoPin) == LOW) {}

                    changeLedStatus(0,0,0);
                    unsigned long finishedReactionTime = millis() - ledOnTime;
                    Serial.println("The Reaction Time test has concluded.");
                    String testResult = String("Your reaction time was ") + finishedReactionTime + " ms.";
                    Serial.println(testResult);
                    Serial.println("Press the green button again anytime to try again.");
                    testActive = false;
                }
            }
        }
        buttonOneLastPressed = millis();
        lastButtonOneState = currentButtonOneState;
    }
}
