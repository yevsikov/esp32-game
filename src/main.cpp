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

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

// Стейти гри (крок 1: тільки стартовий екран, решта стейтів додамо в наступних ітераціях)
enum class GameState {
    START_SCREEN,
    PLAYING,
    GAME_OVER,
};

GameState gameState = GameState::START_SCREEN;

void renderStartScreen() {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("t-rex game");
    display.println("press button");
    display.display();
}

void setup() {
    Serial.begin(115200);
    Wire.begin(PIN_SDA, PIN_SCL);

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
    // наступний крок: неблокуюча обробка кнопки і зміна gameState
}
