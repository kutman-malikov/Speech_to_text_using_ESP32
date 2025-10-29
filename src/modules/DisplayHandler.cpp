#include "modules/DisplayHandler.h"

DisplayHandler::DisplayHandler(int8_t cs, int8_t dc, int8_t rst)
    : tft(cs, dc, rst), _cs(cs), _dc(dc), _rst(rst), _bootLineY(0) {}

void DisplayHandler::begin() {
    Serial.println("[Display] Initializing ILI9341...");
    
    // Ручной reset
    if (_rst >= 0) {
        pinMode(_rst, OUTPUT);
        digitalWrite(_rst, HIGH);
        delay(100);
        digitalWrite(_rst, LOW);
        delay(100);
        digitalWrite(_rst, HIGH);
        delay(200);
    }
    
    tft.begin();
    tft.setRotation(0);  // 240x320 portrait (пины внизу)
    tft.fillScreen(ILI9341_BLACK);
    delay(300);
    
    // Заголовок загрузки
    tft.setTextSize(3);
    tft.setTextColor(ILI9341_CYAN);
    tft.setCursor(30, 30);
    tft.println("ESP32");
    tft.setCursor(50, 60);
    tft.println("STT");
    
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(20, 100);
    tft.println("Speech-to-Text System");
    
    tft.drawFastHLine(20, 120, 200, ILI9341_BLUE);
    
    _bootLineY = 135;
    Serial.println("[Display] OK");
}

void DisplayHandler::showBootStatus(const String& msg) {
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    tft.setCursor(15, _bootLineY);
    tft.print("> ");
    tft.println(msg);
    
    _bootLineY += 22;
    if (_bootLineY > 200) {
        _bootLineY = 85;
        tft.fillRect(20, 85, 280, 140, ILI9341_BLACK);
    }
}

void DisplayHandler::showStatusBar(const String& state, uint16_t color) {
    tft.fillRect(0, 0, 240, 60, color);
    tft.setTextSize(3);
    tft.setTextColor(ILI9341_WHITE, color);
    
    // Центрирование текста
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(state, 0, 0, &x1, &y1, &w, &h);
    int x = (240 - w) / 2;
    
    tft.setCursor(x, 18);
    tft.println(state);
}

void DisplayHandler::showTranscriptionText(const String& text) {
    tft.fillRect(0, 65, 230, 175, ILI9341_BLACK);
    tft.drawRect(5, 65, 230, 250, ILI9341_DARKGREY);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    
    int cursorX = 12;
    int cursorY = 75;
    const int maxWidth = 296;
    const int lineHeight = 18;
    
    String word = "";
    
    for (unsigned int i = 0; i < text.length(); i++) {
        char c = text[i];
        
        if (c == ' ' || c == '\n' || i == text.length() - 1) {
            if (i == text.length() - 1 && c != ' ' && c != '\n') {
                word += c;
            }
            
            int16_t x1, y1;
            uint16_t w, h;
            tft.getTextBounds(word, 0, 0, &x1, &y1, &w, &h);
            
            if (cursorX + w > maxWidth && cursorX > 12) {
                cursorY += lineHeight;
                cursorX = 12;
            }
            
            tft.setCursor(cursorX, cursorY);
            tft.print(word);
            cursorX += w;
            
            if (c == ' ') {
                tft.print(" ");
                cursorX += 6;
            }
            
            if (c == '\n') {
                cursorY += lineHeight;
                cursorX = 12;
            }
            
            word = "";
        } else {
            word += c;
        }
        
        if (cursorY > 220) break;
    }
}

void DisplayHandler::initMainInterface() {
    clearScreen();
    showStatusBar("Ready", ILI9341_GREEN);
    showTranscriptionText("Ready to record...\nPress the button \nto start.");
}

void DisplayHandler::clearScreen() {
    tft.fillScreen(ILI9341_BLACK);
}