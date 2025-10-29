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
    tft.setRotation(1);  // 320x240 landscape
    tft.fillScreen(ILI9341_BLACK);
    delay(300);
    
    // Заголовок загрузки
    tft.setTextSize(3);
    tft.setTextColor(ILI9341_CYAN);
    tft.setCursor(60, 20);
    tft.println("ESP32 STT");
    
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(70, 50);
    tft.println("Speech-to-Text System");
    
    tft.drawFastHLine(20, 70, 280, ILI9341_BLUE);
    
    _bootLineY = 85;
    Serial.println("[Display] OK");
}

void DisplayHandler::showBootStatus(const String& msg) {
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    tft.setCursor(25, _bootLineY);
    tft.print("> ");
    tft.println(msg);
    
    _bootLineY += 22;
    if (_bootLineY > 200) {
        _bootLineY = 85;
        tft.fillRect(20, 85, 280, 140, ILI9341_BLACK);
    }
}

void DisplayHandler::showStatusBar(const String& state, uint16_t color) {
    tft.fillRect(0, 0, 320, 60, color);
    tft.setTextSize(3);
    tft.setTextColor(ILI9341_WHITE, color);
    
    // Центрирование текста
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(state, 0, 0, &x1, &y1, &w, &h);
    int x = (320 - w) / 2;
    
    tft.setCursor(x, 18);
    tft.println(state);
}

void DisplayHandler::showTranscriptionText(const String& text) {
    tft.fillRect(0, 65, 320, 175, ILI9341_BLACK);
    tft.drawRect(5, 65, 310, 170, ILI9341_DARKGREY);
    
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
    showStatusBar("Hold button to record", ILI9341_GREEN);
    showTranscriptionText("Ready to record...\nPress and hold button.");
}

void DisplayHandler::clearScreen() {
    tft.fillScreen(ILI9341_BLACK);
}