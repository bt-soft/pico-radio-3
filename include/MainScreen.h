#ifndef __MAIN_SCREEN_H
#define __MAIN_SCREEN_H

#include "Config.h"
#include "ScreenManager.h"
#include "UIComponents.h"
#include "rtVars.h"

// Példa paraméter struktúra a képernyők közötti adatátadáshoz
struct MainScreenParams {
    uint32_t frequency;
    bool showFrequency;
    String stationName;
};

class MainScreen : public Screen {
  private:
    // UI komponensek
    std::shared_ptr<Label> frequencyLabel;
    std::shared_ptr<Label> stationLabel;
    std::shared_ptr<Button> muteButton;
    std::shared_ptr<Button> menuButton;
    std::shared_ptr<Panel> topPanel;
    std::shared_ptr<Panel> bottomPanel;

    // Adatok
    uint32_t currentFrequency = 0;
    String currentStation = "";
    bool lastMuteState = false; // Layout méretek
    static const uint16_t TOP_PANEL_HEIGHT = 80;
    static const uint16_t BOTTOM_PANEL_HEIGHT = 60;
    static const uint16_t BUTTON_WIDTH = 80;
    static const uint16_t BUTTON_HEIGHT = 40;
    static const uint16_t MARGIN = 10;

  public:
    MainScreen(TFT_eSPI &tft) : Screen(tft, "MainScreen") {
        createComponents();
        layoutComponents();
    }

    // Rotary encoder eseménykezelés felülírása
    virtual bool handleRotary(const RotaryEvent &event) override {
        DEBUG("MainScreen handleRotary: direction=%d, button=%d\n", (int)event.direction, (int)event.buttonState);

        // Frekvencia állítás forgatással
        if (event.direction == RotaryEvent::Direction::Up) {
            adjustFrequency(true);
            return true;
        } else if (event.direction == RotaryEvent::Direction::Down) {
            adjustFrequency(false);
            return true;
        }

        // Mute váltás kattintással
        if (event.buttonState == RotaryEvent::ButtonState::Clicked) {
            toggleMute();
            return true;
        }

        // Ha nem kezeltük, továbbítjuk a szülő implementációnak (gyerekkomponenseknek)
        return Screen::handleRotary(event);
    }

    // Rotary encoder támogatás
    void adjustFrequency(bool increase) {
        // Frekvencia lépés (például 100 kHz FM-ben, 10 kHz AM-ben)
        int step = 100; // kHz - ezt lehetne a sáv alapján állítani

        if (increase) {
            currentFrequency += step;
        } else {
            currentFrequency -= step;
        }

        // Frekvencia határok ellenőrzése
        // FM: 87.5 - 108 MHz, AM: 520 - 1710 kHz
        if (currentFrequency < 87500)
            currentFrequency = 87500;
        if (currentFrequency > 108000)
            currentFrequency = 108000;

        // Si4735 frekvencia beállítása
        // si4735.setFrequency(currentFrequency);

        updateFrequencyDisplay();
    }

    void toggleMute() { onMuteClicked(); }

    virtual ~MainScreen() = default;

  protected:
    void createComponents() {
        // Színsémák
        ColorScheme panelColors = ColorScheme::defaultScheme();
        panelColors.background = TFT_BLACK;
        panelColors.border = TFT_DARKGREY;

        ColorScheme labelColors = ColorScheme::defaultScheme();
        labelColors.background = TFT_TRANSPARENT;
        labelColors.foreground = TFT_CYAN;

        ColorScheme buttonColors = ColorScheme::defaultScheme();
        buttonColors.background = TFT_DARKGREEN;
        buttonColors.foreground = TFT_WHITE;
        buttonColors.pressedBackground = TFT_GREEN;

        // Panelek létrehozása
        topPanel = std::make_shared<Panel>(tft, Rect(0, 0, tft.width(), TOP_PANEL_HEIGHT), panelColors);
        bottomPanel = std::make_shared<Panel>(tft, Rect(0, tft.height() - BOTTOM_PANEL_HEIGHT, tft.width(), BOTTOM_PANEL_HEIGHT), panelColors);

        // Frekvencia címke
        frequencyLabel = std::make_shared<Label>(tft, Rect(MARGIN, MARGIN, 200, 30), "88.5 MHz", labelColors);
        frequencyLabel->setTextSize(3);
        frequencyLabel->setTextDatum(TL_DATUM);

        // Állomás címke
        stationLabel = std::make_shared<Label>(tft, Rect(MARGIN, 45, 200, 25), "Radio Station", labelColors);
        stationLabel->setTextSize(2);
        stationLabel->setTextDatum(TL_DATUM); // Némítás gomb (koordináták a bottomPanel-hez képest relatívak)
        Rect muteButtonRect(MARGIN, MARGIN, BUTTON_WIDTH, BUTTON_HEIGHT);
        DEBUG("Creating MUTE button at: (%d,%d) size: %dx%d (relative to bottomPanel)\n", muteButtonRect.x, muteButtonRect.y, muteButtonRect.width, muteButtonRect.height);
        muteButton = std::make_shared<Button>(tft, muteButtonRect, "MUTE", buttonColors);
        muteButton->setClickCallback([this]() { onMuteClicked(); }); // Menü gomb (koordináták a bottomPanel-hez képest relatívak)
        Rect menuButtonRect(tft.width() - BUTTON_WIDTH - MARGIN, MARGIN, BUTTON_WIDTH, BUTTON_HEIGHT);
        DEBUG("Creating MENU button at: (%d,%d) size: %dx%d (relative to bottomPanel)\n", menuButtonRect.x, menuButtonRect.y, menuButtonRect.width, menuButtonRect.height);
        menuButton = std::make_shared<Button>(tft, menuButtonRect, "MENU", buttonColors);
        menuButton->setClickCallback([this]() { onMenuClicked(); });

        // Komponensek hozzáadása a panelekhez
        topPanel->addChild(frequencyLabel);
        topPanel->addChild(stationLabel);
        bottomPanel->addChild(muteButton);
        bottomPanel->addChild(menuButton);

        // Panelek hozzáadása a képernyőhöz
        addChild(topPanel);
        addChild(bottomPanel);
    }

    void layoutComponents() {
        // Layout frissítése (ha változik a képernyő méret)
        topPanel->setBounds(Rect(0, 0, tft.width(), TOP_PANEL_HEIGHT));
        bottomPanel->setBounds(Rect(0, tft.height() - BOTTOM_PANEL_HEIGHT, tft.width(), BOTTOM_PANEL_HEIGHT));

        // Gombok pozicionálása
        muteButton->setBounds(Rect(MARGIN, tft.height() - BOTTOM_PANEL_HEIGHT + MARGIN, BUTTON_WIDTH, BUTTON_HEIGHT));
        menuButton->setBounds(Rect(tft.width() - BUTTON_WIDTH - MARGIN, tft.height() - BOTTOM_PANEL_HEIGHT + MARGIN, BUTTON_WIDTH, BUTTON_HEIGHT));
    }
    virtual void onActivate() override {
        // Képernyő aktiválásakor
        DEBUG("MainScreen activated\n");
        updateDisplay();
        // Force redraw of all components when screen is activated
        markForRedraw();
        forceChildRedraw();
    }

    virtual void onParametersSet(void *params) override {
        if (params) {
            MainScreenParams *screenParams = static_cast<MainScreenParams *>(params);
            currentFrequency = screenParams->frequency;
            currentStation = screenParams->stationName;

            if (screenParams->showFrequency) {
                updateFrequencyDisplay();
            }
            updateStationDisplay();
        }
    }

    virtual void handleOwnLoop() override {
        // Saját loop logika
        updateRuntimeData();
    }
    virtual void drawSelf() override {
        if (needsRedraw) {
            // Teljes képernyő törlése
            tft.fillScreen(TFT_BLACK);
            needsRedraw = false;
            // Force all child components to redraw after clearing screen
            forceChildRedraw();
        }
    }

    // Helper method to force all child components to redraw
    void forceChildRedraw() {
        // Force the main panels to redraw and all their children
        if (topPanel) {
            topPanel->markForRedraw();
            if (frequencyLabel)
                frequencyLabel->markForRedraw();
            if (stationLabel)
                stationLabel->markForRedraw();
        }
        if (bottomPanel) {
            bottomPanel->markForRedraw();
            if (muteButton)
                muteButton->markForRedraw();
            if (menuButton)
                menuButton->markForRedraw();
        }
    }

  private:
    void updateRuntimeData() {
        // Mute állapot ellenőrzése
        if (rtv::mute != lastMuteState) {
            lastMuteState = rtv::mute;
            updateMuteButton();
        }

        // További runtime adatok frissítése...
        // Frekvencia, jelerősség, stb.
    }

    void updateDisplay() {
        updateFrequencyDisplay();
        updateStationDisplay();
        updateMuteButton();
    }

    void updateFrequencyDisplay() {
        // rtVars-ból frekvencia olvasása és megjelenítése
        String freqText = String(currentFrequency / 1000.0, 1) + " MHz";
        frequencyLabel->setText(freqText);
    }

    void updateStationDisplay() { stationLabel->setText(currentStation.isEmpty() ? "No Station" : currentStation); }

    void updateMuteButton() {
        if (rtv::mute) {
            muteButton->setText("UNMUTE");
            ColorScheme redColors = ColorScheme::defaultScheme();
            redColors.background = TFT_RED;
            redColors.foreground = TFT_WHITE;
            redColors.pressedBackground = TFT_RED;
            muteButton->setColorScheme(redColors);
        } else {
            muteButton->setText("MUTE");
            ColorScheme greenColors = ColorScheme::defaultScheme();
            greenColors.background = TFT_DARKGREEN;
            greenColors.foreground = TFT_WHITE;
            greenColors.pressedBackground = TFT_GREEN;
            muteButton->setColorScheme(greenColors);
        }
    }
    void onMuteClicked() {
        DEBUG("MUTE button clicked! Current mute state: %s\n", rtv::mute ? "true" : "false");
        // Mute állapot váltása
        rtv::mute = !rtv::mute;

        // Itt lehetne a hardver mute pin kezelése is
        digitalWrite(PIN_AUDIO_MUTE, rtv::mute ? HIGH : LOW);

        updateMuteButton();
        DEBUG("MUTE button clicked! New mute state: %s\n", rtv::mute ? "true" : "false");
    }
    void onMenuClicked() {
        DEBUG("MENU button clicked! Switching to MenuScreen\n");
        // Menü képernyőre váltás
        if (screenManager) {
            screenManager->switchToScreen("MenuScreen");
        } else {
            DEBUG("ERROR: screenManager is null!\n");
        }
    }
};

#endif // __MAIN_SCREEN_H
