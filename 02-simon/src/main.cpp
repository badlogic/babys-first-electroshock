#include <Arduino.h>

#define NUM_ELEMENTS 3

#define LED_1 11
#define LED_2 12
#define LED_3 13

#define BUTTON_1 5
#define BUTTON_2 6
#define BUTTON_3 7
#define BUTTON_DEBOUNCE_TIME 50

#define MAX_SEQUENCE 5
#define SEQUENCE_INTERVAL 500
#define SEQUENCE_PAUSE 250

struct Button {
  int pin;
  int state;
  int lastState;
  bool justPressed;
  unsigned long lastDebounceTime;
  const unsigned long debounceDelay = 50;

  Button(int _pin): pin(_pin), state(LOW), lastState(LOW), justPressed(false), lastDebounceTime(0) {
  };

  void init() {
    pinMode(pin, INPUT);
  };

  void update() {
    justPressed = false;
    int reading = digitalRead(pin);

    if (reading != lastState) {
      lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (reading != state) {
        if (state == LOW && reading == HIGH) {
          justPressed = true;
        }
        state = reading;
      }
    }

    lastState = reading;
  };
};

struct AnimationFrame {
  int ledStates[NUM_ELEMENTS];
  int duration;
};

struct Animation {
  unsigned long lastTime;
  int frameIndex;
  AnimationFrame *frames;
  int numFrames;

  Animation(AnimationFrame *_frames): lastTime(-1), frameIndex(0), frames(_frames) {
    while (true) {
      AnimationFrame *frame = &frames[numFrames];
      if (frame->duration == -1) {
        break;
      }
      numFrames++;
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
    if (currentTime - lastTime >= (unsigned long)frames[frameIndex].duration) {
      frameIndex++;
      if (frameIndex >= numFrames) {
        frameIndex = 0;
      }
      lastTime = currentTime;
      for (int i = 0; i < NUM_ELEMENTS; i++) {
        digitalWrite(i + LED_1, frames[frameIndex].ledStates[i]);
      }
    }
  }
};

enum GameState {
  Waiting,
  Playing,
};

GameState gameState = Waiting;
int sequence[MAX_SEQUENCE];
int sequenceIndex = 0;
int playerSequence[MAX_SEQUENCE];
Button buttons[] = { Button(BUTTON_1), Button(BUTTON_2), Button(BUTTON_3) };
int leds[] = { LED_1, LED_2, LED_3 };

AnimationFrame waitingFrames[] = {
  { { HIGH, LOW, LOW }, 500 },
  { { LOW, HIGH, LOW }, 500 },
  { { LOW, LOW, HIGH }, 500 },
  { { 0 }, -1 } ,
};

Animation waitingAnimation(waitingFrames);

AnimationFrame failFrames[] = {
  { { HIGH, LOW, HIGH }, 100 },
  { { LOW, HIGH, LOW }, 100 },
  { { HIGH, HIGH, LOW }, 100 },
  { { LOW, LOW, HIGH }, 100 },
  { { HIGH, LOW, LOW }, 100 },
  { { LOW, HIGH, HIGH }, 100 },
  { { HIGH, HIGH, HIGH }, 100 },
  { { LOW, LOW, LOW }, 100 },
  { { 0 }, -1 } ,
};

Animation failAnimation(failFrames);

AnimationFrame successFrames[] = {
  { { HIGH, LOW, HIGH }, 100 },
  { { LOW, HIGH, LOW }, 100 },
  { { 0 }, -1 } ,
};

Animation successAnimation(successFrames);

AnimationFrame offFrames[] = {
  { { LOW, LOW, LOW }, 500 },
  { { 0 }, -1 } ,
};

Animation offAnimation(offFrames);

AnimationFrame onFrames[] = {
  { { HIGH, HIGH, HIGH }, 500 },
  { { 0 }, -1 } ,
};

Animation onAnimation(onFrames);

void playbackSequence() {
  offAnimation.update();
  delay(1000);
  Serial.println("==== playing sequence");
  Serial.print("Length: ");
  Serial.println(sequenceIndex);
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
  while(millis() - start < 3000 ) {
    if (success) {
      successAnimation.update();
    } else {
      failAnimation.update();
    }
  }
  gameState = Waiting;
}

void readSequence() {
  int playerSequenceIndex = 0;

  Serial.println("==== reading sequence");
  bool done = false;
  while (!done) {
    for (int i = 0; i < NUM_ELEMENTS; i++) {
      buttons[i].update();
      digitalWrite(leds[i], buttons[i].state);

      if (buttons[i].justPressed) {
        playerSequence[playerSequenceIndex] = i;
        playerSequenceIndex++;

        Serial.println(i);
        Serial.print("sequence index: ");
        Serial.println(sequenceIndex);
        Serial.print("player index: ");
        Serial.println(playerSequenceIndex);

        if (playerSequenceIndex == sequenceIndex - 1) {

          while (true) {
            buttons[i].update();
            if (buttons[i].state == LOW) {
              break;
            }
          }

          for (int i = 0; i < playerSequenceIndex; i++) {
            if (sequence[i] != playerSequence[i]) {
              Serial.println("Sequences differ, game over");
              gameOver(false);
              return;
            }
          }

          if (sequenceIndex == MAX_SEQUENCE) {
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
  delay(1000);
  offAnimation.update();
  readSequence();
}

void waitForButtonPress() {
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
    while (true) {
      onAnimation.update();
      bool somePressed = false;
      for (int i = 0; i < NUM_ELEMENTS; i++) {
        buttons[i].update();
        somePressed |= buttons[i].state == HIGH;
      }
      if (!somePressed) {
        break;
      }
    }

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
  if (gameState == Waiting) {
    waitForButtonPress();
  } else if (gameState == Playing) {
    gameLoop();
  }
}