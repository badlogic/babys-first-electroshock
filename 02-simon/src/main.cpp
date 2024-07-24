#include <Arduino.h>

#define NUM_ELEMENTS 3
#define MAX_SEQUENCE 5
#define SEQUENCE_INTERVAL 500
#define SEQUENCE_PAUSE 250
#define LED_1 10

// Debounced button
struct Button {
   int pin;
   int state;
   int lastState;
   bool justPressed;
   unsigned long lastDebounceTime;
   const unsigned long debounceDelay = 50;

   Button(int _pin)
       : pin(_pin), state(HIGH), lastState(HIGH), justPressed(false),
         lastDebounceTime(0) {};

   void init() { pinMode(pin, INPUT_PULLUP); };

   void update() {
      justPressed = false;
      int reading = digitalRead(pin);

      if (reading != lastState) {
         lastDebounceTime = millis();
      }

      if ((millis() - lastDebounceTime) > debounceDelay) {
         if (reading != state) {
            if (state == HIGH && reading == LOW) {
               justPressed = true;
            }
            state = reading;
         }
      }

      lastState = reading;
   };
};

// LED animation frame
struct AnimationFrame {
   int ledStates[NUM_ELEMENTS];
   int duration;
};

// LED Animation consisting of multiple frames
struct Animation {
   unsigned long lastTime;
   int frameIndex;
   AnimationFrame *frames;
   int numFrames;
   int duration;

   Animation(AnimationFrame *_frames)
       : lastTime(-1), frameIndex(0), frames(_frames) {
      while (true) {
         AnimationFrame *frame = &frames[numFrames];
         if (frame->duration == -1)
            break;
         numFrames++;
         duration += frame->duration;
      }
   }

   void reset() {
      lastTime = -1;
      frameIndex = 0;
   }

   void update() {
      if (lastTime == -1) {
         lastTime = millis();
         for (int i = 0; i < NUM_ELEMENTS; i++) {
            digitalWrite(i + LED_1, frames[frameIndex].ledStates[i]);
         }
         return;
      }

      unsigned long currentTime = millis();
      if (currentTime - lastTime >= (unsigned long) frames[frameIndex].duration) {
         frameIndex++;
         if (frameIndex >= numFrames)
            frameIndex = 0;
         lastTime = currentTime;
         for (int i = 0; i < NUM_ELEMENTS; i++) {
            digitalWrite(i + LED_1, frames[frameIndex].ledStates[i]);
         }
      }
   }
};

enum GameState {
   MainMenu,
   Playing,
};

GameState gameState = MainMenu;
int sequence[MAX_SEQUENCE];
int sequenceIndex = 0;
int playerSequence[MAX_SEQUENCE];
Button buttons[] = {Button(5), Button(6), Button(7)};
int leds[] = {LED_1, LED_1 + 1, LED_1 + 2};

AnimationFrame waitingFrames[] = {
        {{HIGH, LOW, LOW}, 250},
        {{LOW, HIGH, LOW}, 250},
        {{LOW, LOW, HIGH}, 250},
        {{0}, -1},
};
Animation waitingAnimation(waitingFrames);

AnimationFrame failFrames[] = {
        {{HIGH, HIGH, HIGH}, 1000},
        {{HIGH, HIGH, LOW}, 1000},
        {{HIGH, LOW, LOW}, 1000},
        {{LOW, LOW, LOW}, 1000},
        {{0}, -1},
};
Animation failAnimation(failFrames);

AnimationFrame successFrames[] = {
        {{HIGH, LOW, HIGH}, 200},
        {{LOW, HIGH, LOW}, 200},
        {{HIGH, LOW, HIGH}, 200},
        {{LOW, HIGH, LOW}, 200},
        {{HIGH, LOW, HIGH}, 200},
        {{LOW, HIGH, LOW}, 200},
        {{HIGH, LOW, HIGH}, 200},
        {{LOW, HIGH, LOW}, 200},
        {{HIGH, LOW, HIGH}, 200},
        {{LOW, HIGH, LOW}, 200},
        {{HIGH, LOW, HIGH}, 200},
        {{LOW, HIGH, LOW}, 200},
        {{HIGH, LOW, HIGH}, 200},
        {{LOW, HIGH, LOW}, 200},
        {{HIGH, LOW, HIGH}, 200},
        {{LOW, HIGH, LOW}, 200},
        {{0}, -1},
};
Animation successAnimation(successFrames);

AnimationFrame offFrames[] = {
        {{LOW, LOW, LOW}, 500},
        {{0}, -1},
};
Animation offAnimation(offFrames);

AnimationFrame onFrames[] = {
        {{HIGH, HIGH, HIGH}, 500},
        {{0}, -1},
};
Animation onAnimation(onFrames);

void waitForNoButtonsPressed() {
   while (true) {
      bool somePressed = false;
      for (int i = 0; i < NUM_ELEMENTS; i++) {
         buttons[i].update();
         somePressed |= buttons[i].state == LOW;
      }
      if (!somePressed) {
         break;
      }
   }
}

void playbackSequence() {
   offAnimation.update();
   delay(1000);
   Serial.println("==== playing sequence");
   for (int i = 0; i < sequenceIndex; i++) {
      offAnimation.update();
      delay(SEQUENCE_PAUSE);
      digitalWrite(sequence[i] + leds[0], HIGH);
      Serial.println(sequence[i]);
      delay(SEQUENCE_INTERVAL);
   }
   sequenceIndex++;
}

void gameOver(bool success) {
   unsigned long start = millis();
   Animation *animation = success ? &successAnimation : &failAnimation;
   animation->reset();
   while (millis() - start < animation->duration) {
      animation->update();
   }
   waitingAnimation.reset();
   gameState = MainMenu;
}

void readSequence() {
   int playerSequenceIndex = 0;

   Serial.println("==== reading sequence");
   bool done = false;
   while (!done) {
      for (int i = 0; i < NUM_ELEMENTS; i++) {
         buttons[i].update();
         digitalWrite(leds[i], !buttons[i].state);

         if (buttons[i].justPressed) {
            playerSequence[playerSequenceIndex] = i;
            Serial.println(i);
            playerSequenceIndex++;

            if (playerSequenceIndex == sequenceIndex - 1) {
               waitForNoButtonsPressed();

               for (int i = 0; i < playerSequenceIndex; i++) {
                  if (sequence[i] != playerSequence[i]) {
                     Serial.println("Sequences differ, failed");
                     gameOver(false);
                     return;
                  }
               }

               if (sequenceIndex == MAX_SEQUENCE) {
                  Serial.println("Sequence matched, success");
                  gameOver(true);
               }

               done = true;
               break;
            }
         }
      }
   }
}

void gameLoop() {
   playbackSequence();
   offAnimation.update();
   delay(500);
   onAnimation.update();
   delay(500);
   offAnimation.update();
   readSequence();
}

void mainMenu() {
   waitingAnimation.update();

   bool justPressed = false;
   for (int i = 0; i < NUM_ELEMENTS; i++) {
      buttons[i].update();
      if (buttons[i].justPressed) {
         justPressed = true;
         break;
      }
   }

   if (justPressed) {
      waitForNoButtonsPressed();

      offAnimation.update();

      for (int i = 0; i < MAX_SEQUENCE; i++) {
         sequence[i] = random(0, 3);
      }
      sequenceIndex = 1;
      gameState = Playing;
      return;
   }
}

void setup() {
   Serial.begin(9600);
   randomSeed(analogRead(0));

   for (int i = 0; i < NUM_ELEMENTS; i++) {
      pinMode(leds[i], OUTPUT);
      buttons[i].init();
   }
}

void loop() {
   if (gameState == MainMenu) {
      mainMenu();
   } else if (gameState == Playing) {
      gameLoop();
   }
}