#ifndef __VOLUME_SCREEN_H
#define __VOLUME_SCREEN_H

#include "ScreenManager.h"
#include "UIComponents.h"

class VolumeScreen : public Screen {
  private:
    std::shared_ptr<Panel> titlePanel;
    std::shared_ptr<Panel> contentPanel;
    std::shared_ptr<Label> titleLabel;
    std::shared_ptr<Label> volumeLabel;
    std::shared_ptr<Label> volumeValueLabel;
    std::shared_ptr<Button> volumeDownButton;
    std::shared_ptr<Button> volumeUpButton;
    std::shared_ptr<Button> muteButton;
    std::shared_ptr<Button> backButton;

    // Volume settings
    int currentVolume = 50; // 0-100%
    int maxVolume = 100;
    int minVolume = 0;
    bool isMuted = false;
    int volumeBeforeMute = 50;

    static const uint16_t TITLE_HEIGHT = 40;
    static const uint16_t BUTTON_HEIGHT = 45;
    static const uint16_t MARGIN = 10;
    static const uint16_t BUTTON_WIDTH = 80;

  public:
    VolumeScreen(TFT_eSPI &tft) : Screen(tft, "VolumeScreen") { createComponents(); }

    virtual ~VolumeScreen() = default;

    // Rotary encoder eseménykezelés felülírása
    virtual bool handleRotary(const RotaryEvent &event) override {
        DEBUG("VolumeScreen handleRotary: direction=%d, button=%d\n", (int)event.direction, (int)event.buttonState);

        // Hangerő állítás forgatással
        if (event.direction == RotaryEvent::Direction::Up) {
            int newVolume = currentVolume + 5;
            DEBUG("Rotary UP: Volume %d -> %d\n", currentVolume, newVolume);
            setVolume(newVolume);
            return true;
        } else if (event.direction == RotaryEvent::Direction::Down) {
            int newVolume = currentVolume - 5;
            DEBUG("Rotary DOWN: Volume %d -> %d\n", currentVolume, newVolume);
            setVolume(newVolume);
            return true;
        }

        // Mute váltás kattintással
        if (event.buttonState == RotaryEvent::ButtonState::Clicked) {
            bool newMute = !isMuted;
            DEBUG("Rotary CLICK: Mute %s -> %s\n", isMuted ? "ON" : "OFF", newMute ? "ON" : "OFF");
            setMuted(newMute);
            return true;
        }

        // Ha nem kezeltük, továbbítjuk a szülő implementációnak (gyerekkomponenseknek)
        return Screen::handleRotary(event);
    }

    // Volume értékek getter/setter
    void setVolume(int volume) {
        int newVolume = constrain(volume, minVolume, maxVolume);
        DEBUG("VolumeScreen setVolume: %d -> %d (constrained)\n", currentVolume, newVolume);
        currentVolume = newVolume;
        updateVolumeDisplay();
    }

    int getVolume() const { return currentVolume; }

    void setMuted(bool muted) {
        isMuted = muted;
        updateVolumeDisplay();
        updateMuteButton();
    }

    bool getMuted() const { return isMuted; }

  protected:
    void createComponents() {
        // Színsémák
        ColorScheme titleColors = ColorScheme::defaultScheme();
        titleColors.background = TFT_NAVY;
        titleColors.foreground = TFT_WHITE;
        titleColors.border = TFT_BLUE;

        ColorScheme panelColors = ColorScheme::defaultScheme();
        panelColors.background = TFT_BLACK;
        panelColors.border = TFT_DARKGREY;

        ColorScheme buttonColors = ColorScheme::defaultScheme();
        buttonColors.background = TFT_DARKGREY;
        buttonColors.foreground = TFT_WHITE;
        buttonColors.pressedBackground = TFT_BLUE;
        ColorScheme backButtonColors = ColorScheme::defaultScheme();
        backButtonColors.background = TFT_RED;
        backButtonColors.foreground = TFT_WHITE;
        backButtonColors.pressedBackground = TFT_MAROON;

        // Cím panel
        titlePanel = std::make_shared<Panel>(tft, Rect(0, 0, tft.width(), TITLE_HEIGHT), titleColors);

        // Cím label
        titleLabel = std::make_shared<Label>(tft, Rect(MARGIN, MARGIN, tft.width() - 2 * MARGIN, TITLE_HEIGHT - 2 * MARGIN), "Volume Settings", titleColors);
        titleLabel->setTextSize(2);
        titleLabel->setTextDatum(MC_DATUM);

        // Tartalom panel
        contentPanel = std::make_shared<Panel>(tft, Rect(0, TITLE_HEIGHT, tft.width(), tft.height() - TITLE_HEIGHT), panelColors);

        // Volume címke
        volumeLabel = std::make_shared<Label>(tft, Rect(MARGIN, TITLE_HEIGHT + MARGIN, tft.width() - 2 * MARGIN, 30), "Volume:", panelColors);
        volumeLabel->setTextSize(2);
        volumeLabel->setTextDatum(ML_DATUM);

        // Volume érték címke
        volumeValueLabel = std::make_shared<Label>(tft, Rect(MARGIN, TITLE_HEIGHT + MARGIN + 35, tft.width() - 2 * MARGIN, 40), String(currentVolume) + "%", panelColors);
        volumeValueLabel->setTextSize(3);
        volumeValueLabel->setTextDatum(MC_DATUM);

        // Volume - gomb
        volumeDownButton = std::make_shared<Button>(tft, Rect(MARGIN, TITLE_HEIGHT + 90, BUTTON_WIDTH, BUTTON_HEIGHT), "Vol-", buttonColors);
        volumeDownButton->setClickCallback([this]() { onVolumeDown(); });

        // Volume + gomb
        volumeUpButton = std::make_shared<Button>(tft, Rect(tft.width() - MARGIN - BUTTON_WIDTH, TITLE_HEIGHT + 90, BUTTON_WIDTH, BUTTON_HEIGHT), "Vol+", buttonColors);
        volumeUpButton->setClickCallback([this]() { onVolumeUp(); });

        // Mute gomb
        muteButton = std::make_shared<Button>(tft, Rect((tft.width() - BUTTON_WIDTH) / 2, TITLE_HEIGHT + 90, BUTTON_WIDTH, BUTTON_HEIGHT), "Mute", buttonColors);
        muteButton->setClickCallback([this]() { onMuteToggle(); });

        // Back gomb
        backButton = std::make_shared<Button>(tft, Rect((tft.width() - 100) / 2, tft.height() - BUTTON_HEIGHT - MARGIN, 100, BUTTON_HEIGHT), "Back", backButtonColors);
        backButton->setClickCallback([this]() { onBack(); });

        // Komponensek összekapcsolása
        titlePanel->addChild(titleLabel);
        contentPanel->addChild(volumeLabel);
        contentPanel->addChild(volumeValueLabel);
        contentPanel->addChild(volumeDownButton);
        contentPanel->addChild(volumeUpButton);
        contentPanel->addChild(muteButton);
        contentPanel->addChild(backButton);

        addChild(titlePanel);
        addChild(contentPanel);

        updateVolumeDisplay();
        updateMuteButton();
    }
    virtual void onActivate() override {
        DEBUG("VolumeScreen activated\n");
        updateVolumeDisplay();
        updateMuteButton();
        // Force redraw of all components when screen is activated
        markForRedraw();
        forceChildRedraw();
    }

    virtual void handleOwnLoop() override {
        // Volume képernyő animációk vagy időzített frissítések
    }
    virtual void drawSelf() override {
        if (needsRedraw) {
            tft.fillScreen(TFT_BLACK);
            needsRedraw = false;
            // Force all child components to redraw after clearing screen
            forceChildRedraw();
        }
    } // Helper method to force all child components to redraw
    void forceChildRedraw() {
        // Force the main panels to redraw and all their children
        if (titlePanel) {
            titlePanel->markForRedraw();
            // Force all children of titlePanel to redraw
            if (titleLabel)
                titleLabel->markForRedraw();
        }
        if (contentPanel) {
            contentPanel->markForRedraw();
            // Force all children of contentPanel to redraw
            if (volumeLabel)
                volumeLabel->markForRedraw();
            if (volumeValueLabel)
                volumeValueLabel->markForRedraw();
            if (volumeDownButton)
                volumeDownButton->markForRedraw();
            if (volumeUpButton)
                volumeUpButton->markForRedraw();
            if (muteButton)
                muteButton->markForRedraw();
            if (backButton)
                backButton->markForRedraw();
        }
    }

  private:
    void onVolumeDown() {
        DEBUG("Volume down pressed\n");
        if (!isMuted && currentVolume > minVolume) {
            setVolume(currentVolume - 5);
            // Itt kellene a tényleges volume változtatás (pl. SI4735)
            // si4735.setVolume(currentVolume);
        }
    }

    void onVolumeUp() {
        DEBUG("Volume up pressed\n");
        if (!isMuted && currentVolume < maxVolume) {
            setVolume(currentVolume + 5);
            // Itt kellene a tényleges volume változtatás (pl. SI4735)
            // si4735.setVolume(currentVolume);
        }
    }

    void onMuteToggle() {
        DEBUG("Mute toggle pressed\n");
        if (isMuted) {
            // Unmute
            setMuted(false);
            setVolume(volumeBeforeMute);
        } else {
            // Mute
            volumeBeforeMute = currentVolume;
            setMuted(true);
            // Itt kellene a tényleges mute (pl. SI4735)
            // si4735.setVolume(0);
        }
    }
    void onBack() {
        DEBUG("Volume screen back pressed, switching to MenuScreen\n");
        if (screenManager) {
            screenManager->switchToScreen("MenuScreen");
        }
    }

    void updateVolumeDisplay() {
        if (volumeValueLabel) {
            String volumeText;
            if (isMuted) {
                volumeText = "MUTED";
            } else {
                volumeText = String(currentVolume) + "%";
            }
            volumeValueLabel->setText(volumeText);
        }
    }
    void updateMuteButton() {
        if (muteButton) {
            ColorScheme colors = ColorScheme::defaultScheme();
            if (isMuted) {
                colors.background = TFT_ORANGE;
                colors.foreground = TFT_BLACK;
                colors.pressedBackground = TFT_YELLOW;
                muteButton->setText("Unmute");
            } else {
                colors.background = TFT_DARKGREY;
                colors.foreground = TFT_WHITE;
                colors.pressedBackground = TFT_BLUE;
                muteButton->setText("Mute");
            }
            muteButton->setColorScheme(colors);
        }
    }
};

#endif // __VOLUME_SCREEN_H
