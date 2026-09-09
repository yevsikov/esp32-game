#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

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

void renderStartScreen() {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("t-rex game");
    display.println("press button");
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
    if (consumeButtonPress() && gameState == GameState::START_SCREEN) {
        gameState = GameState::PLAYING;
        score = 0;
        lastScoreTickMs = millis();
        renderPlayingScreen();
    }

    if (gameState == GameState::PLAYING && millis() - lastScoreTickMs >= SCORE_TICK_MS) {
        lastScoreTickMs += SCORE_TICK_MS;
        score++;
        renderPlayingScreen();
    }
}
