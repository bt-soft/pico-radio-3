#ifndef __INFO_SCREEN_H
#define __INFO_SCREEN_H

#include "ScreenManager.h"
#include "UIComponents.h"
#include "defines.h"

class InfoScreen : public Screen {
  private:
    std::shared_ptr<Panel> titlePanel;
    std::shared_ptr<Panel> contentPanel;
    std::shared_ptr<Panel> buttonPanel;
    std::shared_ptr<Label> titleLabel;
    std::shared_ptr<Label> programNameLabel;
    std::shared_ptr<Label> versionLabel;
    std::shared_ptr<Label> authorLabel;
    std::shared_ptr<Label> buildLabel;
    std::shared_ptr<Label> memoryLabel;
    std::shared_ptr<Button> backButton;

    static const uint16_t TITLE_HEIGHT = 40;
    static const uint16_t BUTTON_HEIGHT = 50;
    static const uint16_t MARGIN = 10;
    static const uint16_t LINE_HEIGHT = 25;

  public:
    InfoScreen(TFT_eSPI &tft) : Screen(tft, "InfoScreen") { createComponents(); }

    virtual ~InfoScreen() = default;

    // Rotary encoder eseménykezelés felülírása
    virtual bool handleRotary(const RotaryEvent &event) override {
        DEBUG("InfoScreen handleRotary: direction=%d, button=%d\n", (int)event.direction, (int)event.buttonState);

        // Vissza gombra kattintással vagy dupla kattintással
        if (event.buttonState == RotaryEvent::ButtonState::Clicked || event.buttonState == RotaryEvent::ButtonState::DoubleClicked) {
            if (screenManager) {
                screenManager->goBack();
            }
            return true;
        }

        // Ha nem kezeltük, továbbítjuk a szülő implementációnak (gyerekkomponenseknek)
        return Screen::handleRotary(event);
    }

  protected:
    void createComponents() {
        // Színsémák
        ColorScheme titleColors = ColorScheme::defaultScheme();
        titleColors.background = TFT_NAVY;
        titleColors.foreground = TFT_WHITE;
        titleColors.border = TFT_BLUE;

        ColorScheme contentColors = ColorScheme::defaultScheme();
        contentColors.background = TFT_BLACK;
        contentColors.border = TFT_DARKGREY;

        ColorScheme labelColors = ColorScheme::defaultScheme();
        labelColors.background = TFT_TRANSPARENT;
        labelColors.foreground = TFT_WHITE;

        ColorScheme buttonColors = ColorScheme::defaultScheme();
        buttonColors.background = TFT_RED;
        buttonColors.foreground = TFT_WHITE;
        buttonColors.pressedBackground = TFT_RED;

        // Panelek
        titlePanel = std::make_shared<Panel>(tft, Rect(0, 0, tft.width(), TITLE_HEIGHT), titleColors);

        contentPanel = std::make_shared<Panel>(tft, Rect(0, TITLE_HEIGHT, tft.width(), tft.height() - TITLE_HEIGHT - BUTTON_HEIGHT), contentColors);

        buttonPanel = std::make_shared<Panel>(tft, Rect(0, tft.height() - BUTTON_HEIGHT, tft.width(), BUTTON_HEIGHT), contentColors);

        // Címke
        titleLabel = std::make_shared<Label>(tft, Rect(MARGIN, MARGIN, tft.width() - 2 * MARGIN, TITLE_HEIGHT - 2 * MARGIN), "Rendszer információ", titleColors);
        titleLabel->setTextSize(2);
        titleLabel->setTextDatum(MC_DATUM);

        // Tartalom címkék
        int yPos = TITLE_HEIGHT + MARGIN;

        programNameLabel = std::make_shared<Label>(tft, Rect(MARGIN, yPos, tft.width() - 2 * MARGIN, LINE_HEIGHT), String("Program: ") + PROGRAM_NAME, labelColors);
        programNameLabel->setTextSize(1);
        yPos += LINE_HEIGHT;

        versionLabel = std::make_shared<Label>(tft, Rect(MARGIN, yPos, tft.width() - 2 * MARGIN, LINE_HEIGHT), String("Verzió: ") + PROGRAM_VERSION, labelColors);
        versionLabel->setTextSize(1);
        yPos += LINE_HEIGHT;

        authorLabel = std::make_shared<Label>(tft, Rect(MARGIN, yPos, tft.width() - 2 * MARGIN, LINE_HEIGHT), String("Szerző: ") + PROGRAM_AUTHOR, labelColors);
        authorLabel->setTextSize(1);
        yPos += LINE_HEIGHT;

        buildLabel = std::make_shared<Label>(tft, Rect(MARGIN, yPos, tft.width() - 2 * MARGIN, LINE_HEIGHT), String("Build: ") + __DATE__ + " " + __TIME__, labelColors);
        buildLabel->setTextSize(1);
        yPos += LINE_HEIGHT;

        memoryLabel = std::make_shared<Label>(tft, Rect(MARGIN, yPos, tft.width() - 2 * MARGIN, LINE_HEIGHT), "Memória: Betöltés...", labelColors);
        memoryLabel->setTextSize(1);

        // Vissza gomb
        backButton = std::make_shared<Button>(tft, Rect(MARGIN, tft.height() - BUTTON_HEIGHT + MARGIN, 100, BUTTON_HEIGHT - 2 * MARGIN), "Vissza", buttonColors);
        backButton->setClickCallback([this]() { onBackClicked(); });

        // Komponensek összekapcsolása
        titlePanel->addChild(titleLabel);

        contentPanel->addChild(programNameLabel);
        contentPanel->addChild(versionLabel);
        contentPanel->addChild(authorLabel);
        contentPanel->addChild(buildLabel);
        contentPanel->addChild(memoryLabel);

        buttonPanel->addChild(backButton);

        addChild(titlePanel);
        addChild(contentPanel);
        addChild(buttonPanel);
    }

    virtual void onActivate() override { updateMemoryInfo(); }

    virtual void handleOwnLoop() override {
        // Memória információ frissítése időnként
        static uint32_t lastMemoryUpdate = 0;
        if (millis() - lastMemoryUpdate >= 2000) { // 2 másodpercenként
            updateMemoryInfo();
            lastMemoryUpdate = millis();
        }
    }

    virtual void drawSelf() override {
        if (needsRedraw) {
            tft.fillScreen(TFT_BLACK);
            needsRedraw = false;
        }
    }

  private:
    void updateMemoryInfo() {
        // Szabad memória lekérdezése (Arduino függvény)
        size_t freeHeap = rp2040.getFreeHeap();
        size_t totalHeap = rp2040.getTotalHeap();
        size_t usedHeap = totalHeap - freeHeap;

        String memText = String("Memória: ") + String(usedHeap) + "/" + String(totalHeap) + " bytes használva";

        memoryLabel->setText(memText);
    }
    void onBackClicked() {
        if (screenManager) {
            screenManager->goBack();
        }
    }
};

#endif // __INFO_SCREEN_H
