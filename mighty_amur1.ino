#include <Adafruit_NeoPixel.h>

#define NUMPIXELS      4
#define R              250
#define G              0
#define B              0

class Part {
  private:
    int forceReadPin;
    Adafruit_NeoPixel light;
    int buzzerWritePin;
  public:
    Part(int forceReadPin, int pixelPin, int buzzerWritePin) {
        this->buzzerWritePin = buzzerWritePin;
        this->forceReadPin = forceReadPin;
        this->light = Adafruit_NeoPixel(
                NUMPIXELS, pixelPin, NEO_GRB + NEO_KHZ800
        );
    }

    void setup() {
        pinMode(buzzerWritePin, OUTPUT);
        pinMode(light.getPin(), OUTPUT);
    }

    void blink(int r, int g, int b) {
        light.fill(light.Color(r, g, b), 0, 0);
        light.show();
    }

    void blink(unsigned long int rgb) {
        light.fill(rgb, 0, 0);
        light.show();
    }

    void makeSound(int hz) {
        tone(buzzerWritePin, hz);
    }

    void clear() {
        light.clear();
        light.show();
        noTone(buzzerWritePin);
    }

    int readForce() {
        unsigned int fsrReading = analogRead(forceReadPin);
        // 466 is the max possible value
        return fsrReading * 100 / 466;
    }

    int readMaxForce(int delay_ms) {
        int total_iterations = delay_ms / 20;
        int maxForce = 0;

        for (int i = 0; i < total_iterations; i++) {
            int force = readForce();
            if (force > maxForce) {
                maxForce = force;
            }
            delay(20);
        }
        return maxForce;
    }

    void displayFeedback(int force, int mode) {
        unsigned long int color = (force > 30) ? light.Color(0, 255, 0) : light.Color(255, 255, 0);
        int sound = (force > 30) ? 600 : 250;

        if (mode == 0 or mode == 1) blink(color);
        if (mode == 0 or mode == 2) makeSound(sound);
    }
};

class Controls {
  private:
    int increaseTimerPin;
    int decreaseTimerPin;
    int changeModePin;
    int timerDelay;
    int mode;

    void incrementMode() {
        mode = (mode + 1) % 3;
    }

    void incrementTimer() {
        timerDelay += 100;
        if (timerDelay > 3000) {
            timerDelay = 3000;
        }
    }

    void decrementTimer() {
        timerDelay -= 100;
        if (timerDelay < 100) {
            timerDelay = 100;
        }
    }

  public:
    Controls(int increaseTimerPin, int decreaseTimerPin, int changeModePin) {
        this->increaseTimerPin = increaseTimerPin;
        this->decreaseTimerPin = decreaseTimerPin;
        this->changeModePin = changeModePin;
        this->timerDelay = 1000;
        this->mode = 0;
    }

    int getMode() {
        return mode;
    }

    int getTimerDelay() {
        return timerDelay;
    }

    void setup() {
        pinMode(increaseTimerPin, INPUT);
        pinMode(decreaseTimerPin, INPUT);
        pinMode(changeModePin, INPUT);
    }

    void waitForInput(int delay_ms) {
      int startTime = millis();
      
      int increaseTimerLastButtonState = 0;
      int decreaseTimerLastButtonState = 0;
      int changeModeLastButtonState = 0;
      
      while (millis() - startTime < delay_ms) {
        int increaseTimerButtonState = digitalRead(increaseTimerPin);
        int decreaseTimerButtonState = digitalRead(decreaseTimerPin);
        int changeModeButtonState = digitalRead(changeModePin);
        
        if (increaseTimerButtonState == HIGH && increaseTimerLastButtonState == LOW) {
          incrementTimer();
          Serial.print("Increment delay to : ");
          Serial.println(timerDelay);
        }
        if (decreaseTimerButtonState == HIGH && decreaseTimerLastButtonState == LOW) {
          decrementTimer();
          Serial.print("Decrement delay to : ");
          Serial.println(timerDelay);
        }
        if (changeModeButtonState == HIGH && changeModeLastButtonState == LOW) {
          incrementMode();
          Serial.print("Switched to mode : ");
          Serial.println(mode);
        }
        increaseTimerLastButtonState = increaseTimerButtonState;
        decreaseTimerLastButtonState = decreaseTimerButtonState;
        changeModeLastButtonState = changeModeButtonState;
      }
    }
};


Controls controls = Controls(10, 11, 12);

int partsLen = 4;
Part parts[] = {
    Part(3, 2, 6),
    Part(2, 3, 7),
    Part(1, 4, 8),
    Part(0, 5, 9),
};


// Code
void setup() {
    Serial.begin(9600);
    
    for (int i = 0; i < partsLen; i++) {
        parts[i].setup();
    }
    controls.setup();
    randomSeed(analogRead(0));
}

void loop() {
    int randPartI = random(0, partsLen);
    Part part = parts[randPartI];
    int current_mode = controls.getMode();
    int timerDelay = controls.getTimerDelay();
    
    Serial.print("New punch : ");
    Serial.print(randPartI);
    Serial.print(" | Delay : ");
  	Serial.print(timerDelay);
    Serial.print(" | Mode : ");
  	Serial.println(current_mode);
  
    if (current_mode == 0 or current_mode == 1) part.blink(R, G, B);
    if (current_mode == 0 or current_mode == 2) part.makeSound(440);
    int force = part.readMaxForce(timerDelay);
    part.clear();

    part.displayFeedback(force, current_mode);
    controls.waitForInput(2000);
    part.clear();
}