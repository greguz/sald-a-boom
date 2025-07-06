#include <SD.h>
#include <ArduinoSound.h>

// MAX4466 electret microphone
#define PIN_MIC         A1

// 10 KOhm potentiometer
#define PIN_VOLUME      A2

// 0 to 4095
#define MIC_SILENCE     2047

// 20Hz delta, 50%
#define MIC_THRESHOLD   2048

// Milliseconds between buttons polling
#define POLL_TIMEOUT    50

// Milliseconds before CONFIG mode
#define CONFIG_TIMEOUT  3000

// Buttons grid digital outputs
#define PIN_COL_0       6
#define PIN_COL_1       7

// Buttons grid digital inputs (pullup)
#define PIN_ROW_0       11
#define PIN_ROW_1       10
#define PIN_ROW_2       9
#define PIN_ROW_3       8

// Running modes
typedef enum {
  CONFIG,
  KEYBOARD,
  SAX,
  LOOP
} Mode;

// Buttons grid status
uint8_t buttonsState = 0x00;

// True when buttons was updated
bool buttonsChanged = false;

// 0 or 1 or 2 or 3
uint8_t bank = 0;

// Current volume (percentage)
uint8_t volume = 20;

//
char filename[13] = "TRACK_99.WAV";

// Playing wave file
SDWaveFile file;

// Current mode
Mode mode = KEYBOARD;

void setup() {
  // Set the resolution to 12 bits (0-4095)
  analogReadResolution(12);

  // Setup buttons grid inputs
  pinMode(PIN_ROW_0, INPUT_PULLUP);
  pinMode(PIN_ROW_1, INPUT_PULLUP);
  pinMode(PIN_ROW_2, INPUT_PULLUP);
  pinMode(PIN_ROW_3, INPUT_PULLUP);

  // Setup buttons grid outputs
  pinMode(PIN_COL_0, OUTPUT);
  pinMode(PIN_COL_1, OUTPUT);
  digitalWrite(PIN_COL_0, HIGH);
  digitalWrite(PIN_COL_1, HIGH);

  digitalWrite(LED_BUILTIN, HIGH); // light equals problems
  delay(10);

  if (!SD.begin()) {
    while(1);
  }

  // Set volume
  AudioOutI2S.volume(volume);

  // Notify ready status
  digitalWrite(LED_BUILTIN, LOW);
  delay(10);

  // Serial.begin(9600);
  // delay(10);
}

void loop() {
  bool polled = pollButtons();

  switch (mode) {
  case CONFIG:
    handleConfigMode();
    break;
  case KEYBOARD:
    handleKeyboardMode();
    break;
  case SAX:
    handleSaxMode(polled);
    break;
  case LOOP:
    handleLoopMode();
    break;
  }

  buttonsChanged = false;
}

void changeMode(Mode value) {
  stopPlaying();

  if (value == CONFIG) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }

  playNotification();
  mode = value;
}

void handleConfigMode() {
  if (buttonsChanged) {
    switch (buttonsState) {
    case 0x11:
      bank = 0;
      changeMode(KEYBOARD);
      break;
    case 0x21:
      bank = 1;
      changeMode(KEYBOARD);
      break;
    case 0x41:
      bank = 2;
      changeMode(KEYBOARD);
      break;
    case 0x81:
      bank = 3;
      changeMode(KEYBOARD);
      break;

    case 0x12:
      bank = 0;
      changeMode(SAX);
      break;
    case 0x22:
      bank = 1;
      changeMode(SAX);
      break;
    case 0x42:
      bank = 2;
      changeMode(SAX);
      break;
    case 0x82:
      bank = 3;
      changeMode(SAX);
      break;

    case 0x14:
      bank = 0;
      changeMode(LOOP);
      break;
    case 0x24:
      bank = 1;
      changeMode(LOOP);
      break;
    case 0x44:
      bank = 2;
      changeMode(LOOP);
      break;
    case 0x84:
      bank = 3;
      changeMode(LOOP);
      break;

    // case 0x48:
    //   if (volume < 80) {
    //     volume += 10;
    //   }
    //   playNotification();
    //   break;
    // case 0x88:
    //   if (volume > 10) {
    //     volume -= 10;
    //   }
    //   playNotification();
    //   break;

    }
  }
}

void handleSaxMode(bool polled) {
  static int min = MIC_SILENCE;
  static int max = MIC_SILENCE;

  int value = analogRead(PIN_MIC);
  if (value < min) {
    min = value;
  }
  if (value > max) {
    max = value;
  }

  if (polled) {
    if (max - min >= MIC_THRESHOLD) {
      if (buttonsChanged || !AudioOutI2S.isPlaying()) {
        playTrack();
      }
    } else {
      stopPlaying();
    }

    min = MIC_SILENCE;
    max = MIC_SILENCE;
  }
}

void handleKeyboardMode() {
  if (buttonsState > 0) {
    if (buttonsChanged || !AudioOutI2S.isPlaying()) {
      playTrack();
    }
  } else {
    stopPlaying();
  }
}

void handleLoopMode() {
  static uint8_t previousState = 0x00;

  if (previousState > 0 && !AudioOutI2S.isPlaying() && !AudioOutI2S.isPaused()) {
    previousState = 0x00;
  }

  if (buttonsChanged) {
    if (buttonsState > 0) {
      if (buttonsState == previousState) {
        if (AudioOutI2S.isPlaying()) {
          AudioOutI2S.pause();
        } else if (AudioOutI2S.isPaused()) {
          AudioOutI2S.resume();
        }
      } else {
        playTrack();
      }

      previousState = buttonsState;
    }
  }
}

bool pollButtons() {
  static unsigned long sinceConfig = 0;
  static unsigned long sincePoll = 0;

  // Limit polling
  unsigned long now = millis();
  if (now - sincePoll < POLL_TIMEOUT) {
    return false;
  }

  // Restart polling counter
  sincePoll = now;

  // Current (local) buttons state
  uint8_t state = 0x00;

  digitalWrite(PIN_COL_0, LOW);
  delay(5);

  if (!digitalRead(PIN_ROW_3)) {
    state = 0x08;
  } else if (!digitalRead(PIN_ROW_2)) {
    state = 0x04;
  } else if (!digitalRead(PIN_ROW_1)) {
    state = 0x02;
  } else if (!digitalRead(PIN_ROW_0)) {
    state = 0x01;
  }

  digitalWrite(PIN_COL_0, HIGH);
  digitalWrite(PIN_COL_1, LOW);
  delay(5);

  if (!digitalRead(PIN_ROW_3)) {
    state |= 0x80;
  } else if (!digitalRead(PIN_ROW_2)) {
    state |= 0x40;
  } else if (!digitalRead(PIN_ROW_1)) {
    state |= 0x20;
  } else if (!digitalRead(PIN_ROW_0)) {
    state |= 0x10;
  }

  digitalWrite(PIN_COL_1, HIGH);
  delay(5);

  // Sync global status
  buttonsChanged = state != buttonsState;
  buttonsState = state;

  // Handle CONFIG mode request
  if (mode != CONFIG && buttonsState == 0x18) {
    if (buttonsChanged) {
      sinceConfig = now;
    } else if (now - sinceConfig >= CONFIG_TIMEOUT) {
      changeMode(CONFIG);
    }
  }

  int potentiometer = analogRead(PIN_VOLUME);
  int v = (double)potentiometer / 4096 * 100;
  if (v > 100) {
    v = 100;
  } else if (v < 0) {
    v = 0;
  }

  if (volume != v) {
    volume = v;
    AudioOutI2S.volume(volume);
  }

  return true;
}

void stopPlaying() {
  // If you stop a paused track audio freeze
  if (AudioOutI2S.isPaused()) {
    AudioOutI2S.resume();
  }
  if (AudioOutI2S.isPlaying()) {
    AudioOutI2S.stop();
  }
}

uint8_t getTrack() {
  switch (buttonsState) {
  case 0x00:
    return 0; // no buttons
  case 0x01:
    return 1;
  case 0x02:
    return 2;
  case 0x04:
    return 3;
  case 0x08:
    return 4;
  case 0x10:
    return 5;
  case 0x20:
    return 6;
  case 0x40:
    return 7;
  case 0x80:
    return 8;
  default:
    return 9; // multiple buttons pressed at the same time
  }
}

void playTrack() {
  uint8_t track = getTrack();
  if (track >= 1 && track <= 8) {
    sprintf(filename, "TRACK_%u%u.WAV", bank, getTrack());
    justPlay(mode == LOOP);
  }
}

void playNotification() {
  strcpy(filename, "TRACK_99.WAV");
  justPlay(false);
  while (AudioOutI2S.isPlaying()) {
    // while for audio to play
  }
}

void justPlay(bool loop) {
  stopPlaying();

  // Serial.print("bank=");
  // Serial.print(bank);
  // Serial.print(" track=");
  // Serial.print(getTrack());
  // Serial.print(" filename=");
  // Serial.println(filename);

  file = SDWaveFile(filename);

  if (file && volume > 0) {
    if (AudioOutI2S.canPlay(file)) {
      if (loop) {
        AudioOutI2S.loop(file);
      } else {
        AudioOutI2S.play(file);
      }

      delay(5);
    }
  }
}
