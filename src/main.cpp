#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


static constexpr int OLED_WIDTH = 128;
static constexpr int OLED_HEIGHT = 64;
static constexpr int OLED_RESET = -1;
static constexpr uint8_t OLED_ADDR = 0x3C;

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

// BCD конвертація
uint8_t bcd_to_dec(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

uint8_t dec_to_bcd(uint8_t dec) {
    return ((dec / 10) << 4) | (dec % 10);
}


// Ініціалізація OLED дисплея
void init_oled() {
    uint8_t init_cmds[] = {
        0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00,
        0x40, 0x8D, 0x14, 0x20, 0x00, 0xA1, 0xC8,
        0xDA, 0x12, 0x81, 0xCF, 0xD9, 0xF1, 0xDB,
        0x40, 0xA4, 0xA6, 0xAF
    };
   
    Wire.beginTransmission(OLED_ADDR);
    Wire.write(0x00);  // Control Byte
    for (int i = 0; i < sizeof(init_cmds); i++) {
        Wire.write(init_cmds[i]);
    }
    Wire.endTransmission();
}

// Виведення часу на OLED (спрощено - потребує бібліотеки)
void print_time_oled(const char* text)  {

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
	

    display.setTextSize(1);
    display.setCursor(0, 48);
    display.println(text);
    display.display();


}

void setup() {
    Serial.begin(115200);
    Wire.begin(8, 9); 
    
    //init_oled();
    delay(100);

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println("SSD1306 init failed");
    }

 
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("RTC display starting...");
    display.display();

    Serial.println("System initialized!");
}

void loop() {

    // Виведення на OLED (потребує бібліотеки)
    print_time_oled("test1");

    delay(2000);

    print_time_oled("test222");

    delay(3000);

}
