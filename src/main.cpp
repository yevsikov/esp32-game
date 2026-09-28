#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <string.h>
#include "ground_patterns.h"
#include "dino_sprites.h"
#include "cactus_sprites.h"
#include "start_splash.h"
// #include "sounds.h"

static constexpr int OLED_WIDTH = 128;
static constexpr int OLED_HEIGHT = 64;
static constexpr int OLED_RESET = -1;
static constexpr uint8_t OLED_ADDR = 0x3C;

static constexpr int PIN_SDA = 8;
static constexpr int PIN_SCL = 9;
static constexpr int PIN_BUTTON = 4;
static constexpr unsigned long DEBOUNCE_DELAY_MS = 50;

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

// Стейти гри (крок 3: додано лічильник очок під час PLAYING)
enum class GameState {
    START_SCREEN,
    PLAYING,
    GAME_OVER,
};

GameState gameState = GameState::START_SCREEN;

// Кнопка підтягнута резистором до GND, тому HIGH = натиснута
int lastRawButtonState = LOW;
int debouncedButtonState = LOW;
unsigned long lastDebounceTime = 0;

unsigned long score = 0;
unsigned long lastScoreTickMs = 0;
static constexpr unsigned long SCORE_TICK_MS = 1000;

int trackPatternIndex = 0;
int trackCharIndex = 0;

static constexpr int TRACK_CHAR_PX = 6; // ширина символу шрифту розміру 1
static constexpr int TRACK_VISIBLE_CHARS = OLED_WIDTH / TRACK_CHAR_PX;
static constexpr int TRACK_Y = 48;
static constexpr unsigned long TRACK_TICK_MS = 100;
static constexpr int TRACK_STEP_PX = 2;
static constexpr int CACTUS_Y = TRACK_Y - CACTUS_SPRITE_HEIGHT; // кактус стоїть на лінії землі

char trackBuffer[TRACK_VISIBLE_CHARS + 1];
int trackPixelOffset = 0;
unsigned long lastTrackTickMs = 0;

// Динозавр (крок 8: справжній бітмап-спрайт замість літери)
enum class DinoState {
    ON_GROUND,
    JUMPING,
};

DinoState dinoState = DinoState::ON_GROUND;

static constexpr int DINO_X = 4;
static constexpr int DINO_GROUND_Y = TRACK_Y - DINO_SPRITE_HEIGHT; // сидить одразу над рядком землі
static constexpr int DINO_JUMP_OFFSET_PX = 16;
static constexpr unsigned long JUMP_DURATION_MS = 1400;
unsigned long jumpStartMs = 0;
int dinoY = DINO_GROUND_Y;

// крок 9: плавний рух по Y під час стрибка (синусоїда замість миттєвого стрибка)
static constexpr unsigned long DINO_ANIM_TICK_MS = 30;
unsigned long lastDinoAnimTickMs = 0;

// крок 9: анімація бігу - чергування кадрів, поки динозавр стоїть на землі
static constexpr unsigned long RUN_FRAME_TOGGLE_MS = 200;
unsigned long lastRunToggleMs = 0;
bool runFrameToggle = false;

// Видає наступний символ треку, перебираючи патерни підряд по колу
char nextTrackChar() {
    const char* pattern = GROUND_PATTERNS[trackPatternIndex];
    char c = pattern[trackCharIndex];
    trackCharIndex++;
    if (pattern[trackCharIndex] == '\0') {
        trackCharIndex = 0;
        trackPatternIndex = (trackPatternIndex + 1) % GROUND_PATTERNS_COUNT;
    }
    return c;
}

bool cactusHitsDinoCenter() {
    int dinoCenterX = DINO_X + DINO_SPRITE_WIDTH / 2;

    for (int i = 0; i < TRACK_VISIBLE_CHARS; i++) {
        if (trackBuffer[i] == '*') {
            int cactusX = i * TRACK_CHAR_PX - trackPixelOffset;
            if (cactusX <= dinoCenterX && dinoCenterX < cactusX + CACTUS_SPRITE_WIDTH) {
                return true;
            }
        }
    }

    return false;
}

void initTrack() {
    trackPatternIndex = 0;
    trackCharIndex = 0;

    for (int i = 0; i < TRACK_VISIBLE_CHARS; i++) {
        trackBuffer[i] = nextTrackChar();
    }
    trackBuffer[TRACK_VISIBLE_CHARS] = '\0';
    trackPixelOffset = 0;
}

// Зсуває трек на TRACK_STEP_PX пікселів; коли назбирали цілий символ - скидаємо буфер на один символ вліво
void advanceTrack() {
    trackPixelOffset += TRACK_STEP_PX;
    if (trackPixelOffset >= TRACK_CHAR_PX) {
        trackPixelOffset -= TRACK_CHAR_PX;
        memmove(trackBuffer, trackBuffer + 1, TRACK_VISIBLE_CHARS - 1);
        trackBuffer[TRACK_VISIBLE_CHARS - 1] = nextTrackChar();
    }
}

void renderStartScreen() {
    display.clearDisplay();
    display.drawBitmap(0, 0, START_SPLASH, START_SPLASH_WIDTH, START_SPLASH_HEIGHT, SSD1306_WHITE);
    display.display();
}

void renderPlayingScreen() {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);

    display.setCursor(0, 0);
    display.println("PLAYING");

    char scoreText[8];
    snprintf(scoreText, sizeof(scoreText), "%lu", score);
    int16_t x1, y1;
    uint16_t textWidth, textHeight;
    display.getTextBounds(scoreText, 0, 0, &x1, &y1, &textWidth, &textHeight);
    display.setCursor(OLED_WIDTH - textWidth - 2, 0);
    display.println(scoreText);

    // крок 9.5: '_' взагалі не малюємо (порожня дорога), '*' замінюємо на спрайт кактуса
    for (int i = 0; i < TRACK_VISIBLE_CHARS; i++) {
        if (trackBuffer[i] == '*') {
            int cactusX = i * TRACK_CHAR_PX - trackPixelOffset;
            display.drawBitmap(cactusX, CACTUS_Y, CACTUS_SPRITE, CACTUS_SPRITE_WIDTH, CACTUS_SPRITE_HEIGHT, SSD1306_WHITE);
        }
    }

    const uint8_t* dinoFrame;
    if (dinoState == DinoState::JUMPING) {
        dinoFrame = DINO_FRAME_JUMP;
    } else {
        dinoFrame = runFrameToggle ? DINO_FRAME_RUN : DINO_FRAME_GROUND;
    }
    display.drawBitmap(DINO_X, dinoY, dinoFrame, DINO_SPRITE_WIDTH, DINO_SPRITE_HEIGHT, SSD1306_WHITE);

    display.display();
}

void renderGameOverScreen() {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("GAME OVER");
    display.print("score: ");
    display.println(score);
    display.display();
}

// Повертає true один раз в момент, коли дебаунснута кнопка перейшла в "натиснута"
bool consumeButtonPress() {
    int raw = digitalRead(PIN_BUTTON);

    if (raw != lastRawButtonState) {
        lastDebounceTime = millis();
        lastRawButtonState = raw;
    }

    bool pressedEdge = false;
    if (millis() - lastDebounceTime > DEBOUNCE_DELAY_MS && raw != debouncedButtonState) {
        debouncedButtonState = raw;
        pressedEdge = (debouncedButtonState == HIGH);
    }

    return pressedEdge;
}

void setup() {
    Serial.begin(115200);
    Wire.begin(PIN_SDA, PIN_SCL);
    pinMode(PIN_BUTTON, INPUT);

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println("SSD1306 init failed");
        while (true) {
            delay(1000);
        }
    }

    renderStartScreen();
    Serial.println("System initialized!");
}

void loop() {
    bool buttonPressed = consumeButtonPress();
    GameState stateBeforeInput = gameState;

    if (buttonPressed && stateBeforeInput == GameState::START_SCREEN) {
        gameState = GameState::PLAYING;
        score = 0;
        dinoState = DinoState::ON_GROUND;
        dinoY = DINO_GROUND_Y;
        runFrameToggle = false;
        lastScoreTickMs = millis();
        lastTrackTickMs = millis();
        lastRunToggleMs = millis();
        initTrack();
        renderPlayingScreen();
    } else if (buttonPressed && stateBeforeInput == GameState::PLAYING && dinoState == DinoState::ON_GROUND) {
        dinoState = DinoState::JUMPING;
        jumpStartMs = millis();
        lastDinoAnimTickMs = millis();
        renderPlayingScreen();
    } else if (buttonPressed && stateBeforeInput == GameState::GAME_OVER) {
        gameState = GameState::START_SCREEN;
        renderStartScreen();
    }

    if (gameState == GameState::PLAYING) {
        bool needsRedraw = false;

        if (millis() - lastScoreTickMs >= SCORE_TICK_MS) {
            lastScoreTickMs += SCORE_TICK_MS;
            score++;
            needsRedraw = true;
        }

        if (dinoState == DinoState::JUMPING) {
            if (millis() - lastDinoAnimTickMs >= DINO_ANIM_TICK_MS) {
                lastDinoAnimTickMs += DINO_ANIM_TICK_MS;
                unsigned long elapsed = millis() - jumpStartMs;
                if (elapsed >= JUMP_DURATION_MS) {
                    dinoState = DinoState::ON_GROUND;
                    dinoY = DINO_GROUND_Y;
                } else {
                    float t = (float)elapsed / (float)JUMP_DURATION_MS;
                    dinoY = DINO_GROUND_Y - (int)(DINO_JUMP_OFFSET_PX * sinf(PI * t));
                }
                needsRedraw = true;
            }
        } else if (millis() - lastRunToggleMs >= RUN_FRAME_TOGGLE_MS) {
            lastRunToggleMs += RUN_FRAME_TOGGLE_MS;
            runFrameToggle = !runFrameToggle;
            needsRedraw = true;
        }

        if (millis() - lastTrackTickMs >= TRACK_TICK_MS) {
            lastTrackTickMs += TRACK_TICK_MS;
            advanceTrack();
            needsRedraw = true;

            // колізія спрацьовує, коли кактус доходить до середини динозавра
            bool obstacleAtDino = cactusHitsDinoCenter();
            if (obstacleAtDino && dinoState == DinoState::ON_GROUND) {
                gameState = GameState::GAME_OVER;
            }
        }

        if (gameState == GameState::GAME_OVER) {
            renderGameOverScreen();
        } else if (needsRedraw) {
            renderPlayingScreen();
        }
    }
}
