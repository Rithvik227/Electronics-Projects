# Python Software Twin of the C++ Script for Project 1 for testing using OOP
from abc import ABC, abstractmethod

class PatientInterface(ABC):
    @abstractmethod
    def get_component_name(self):
        pass

class CallButton(PatientInterface):
    def __init__(self, buttonPin, debounceDelay):
        print(f"The button is assigned to pin: {buttonPin}")
        print(f"The button's debounce delay is set to: {debounceDelay}")
        self._buttonLastPressed = 0
        self._lastButtonState = False
        self._currentButtonState = False

    def readButtonCurrentState(self):
        return self._currentButtonState

    def readButtonLastState(self):
        return self._lastButtonState

    def simulatePress(self):
        self._currentButtonState = not self._currentButtonState

    def updateHistory(self):
        self._lastButtonState = self._currentButtonState

    def get_component_name(self):
        return "Pushbutton"

class AlertLED(PatientInterface):
    def __init__(self, ledPin):
        print(f"The led is assigned to pin: {ledPin}")
        self._ledState = False

    def readLedState(self):
        return self._ledState

    def turnOnLed(self):
        self._ledState = True
        print ("Emergency Button Pressed. Emergency LED Activated")

    def turnOffLed(self):
        self._ledState = False
        print ("Emergency Button Depressed. Emergency LED Deactivated")

    def get_component_name(self):
        return "Emergency LED Indicator"

if __name__ == "__main__":
    button = CallButton(2,50)
    led = AlertLED(3)

    button.simulatePress()
    if button.readButtonCurrentState() != button.readButtonLastState():
        if button.readButtonCurrentState() == True:
            if led.readLedState() == False:
                led.turnOnLed()
            else:
                led.turnOffLed()

        print("Button was pressed. Now updating buttonLastPressed (in theory) and lastButtonState")
        button.updateHistory()





