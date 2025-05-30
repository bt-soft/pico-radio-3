#ifndef __FM_SCREEN_H
#define __FM_SCREEN_H

#include "uicomponents/UIScreen.h"

// Példa paraméter struktúra a képernyők közötti adatátadáshoz
struct FMScreenParams {
    uint32_t frequency;
    bool showFrequency;
    String stationName;
};

class FMScreen : public UIScreen {

  public:
    // Képernyő neve konstansként
    static constexpr const char *SCREEN_NAME = "FMScreen";

  private:
    // UI komponens példányok
    // std::shared_ptr<UIButton> menuButton;

  public:
    FMScreen(TFT_eSPI &tft) : UIScreen(tft, FMScreen::SCREEN_NAME) { layoutComponents(); }
    virtual ~FMScreen() = default;

    // Rotary encoder eseménykezelés felülírása
    virtual bool handleRotary(const RotaryEvent &event) override {
        DEBUG("FMScreen handleRotary: direction=%d, button=%d\n", (int)event.direction, (int)event.buttonState);

        // Frekvencia állítás forgatással
        if (event.direction == RotaryEvent::Direction::Up) {
            DEBUG("FMScreen: Rotary Up\n");
            return true;
        } else if (event.direction == RotaryEvent::Direction::Down) {
            DEBUG("FMScreen: Rotary Down\n");
            return true;
        }

        // Rotary gombnyomás kezelése
        if (event.buttonState == RotaryEvent::ButtonState::Clicked) {
            DEBUG("FMScreen: Rotary Clicked\n");
            return true;
        }

        // Ha nem kezeltük, továbbítjuk a szülő implementációnak (gyerekkomponenseknek)
        return UIScreen::handleRotary(event);
    }

    // loop hívás felülírása
    virtual void handleOwnLoop() override {
        // Saját loop logika
    }

    /**
     * @brief Kirajzolja a képernyő saját tartalmát.
     * Ez a metódus arra szolgál, hogy a kompozit komponens maga rajzoljon ki tartalmat, mielőtt a gyerekkomponenseit kirajzolná.
     */
    virtual void drawSelf() override {
        // Szöveg középre igazítása
        tft.setTextDatum(MC_DATUM);                        // Middle Center datum
        tft.setTextColor(TFT_WHITE, TFT_COLOR_BACKGROUND); // Fehér szöveg, fekete háttérrel
        tft.setTextSize(2);                                // Tetszőleges méret

        // Szöveg kirajzolása a képernyő közepére
        tft.drawString(SCREEN_NAME, tft.width() / 2, tft.height() / 2);

        // Alapértelmezett szöveg igazítás visszaállítása, ha szükséges más komponenseknek
        // tft.setTextDatum(TL_DATUM); // Top Left datum (gyakori alapértelmezés)
    }

  private:
    // UI komponensek létrehozása és elhelyezése
    void layoutComponents() {
        // Például gombok, kijelzők létrehozása
        // menuButton = std::make_shared<UIButton>(tft, "MenuButton", 10, 10, 100, 50, "Menu");
    }
};

#endif // __FM_SCREEN_H