#ifndef __FM_SCREEN_H
#define __FM_SCREEN_H

#include "uicomponents/UIButton.h"    // Hozzáadva a UIButton definíciójához
#include "uicomponents/UIComponent.h" // Szükséges a ColorScheme-hez és Rect-hez
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
    std::shared_ptr<UIButton> button1;
    std::shared_ptr<UIButton> button2;
    std::shared_ptr<UIButton> button3;

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
    void handleButton1Event(const UIButton::ButtonEvent &event) {
        DEBUG("FMScreen: Button 1 event! ID: %d, Label: '%s', State: %d\n",
              event.id, event.label.c_str(), (int)event.state);

        if (event.state == UIButton::ButtonState::Pressed) {
            // Gomb 1 megnyomva
            tft.setTextColor(TFT_YELLOW); // Csak példa, a drawSelf felülírhatja
            tft.drawString("Gomb 1 megnyomva!", tft.width() / 2, tft.height() / 2 + 20);
            // Itt végezheted el a kívánt műveletet
            // Fontos: Ha a UI-t módosítod, lehet, hogy markForRedraw()-t kell hívnod
            // a gombon vagy a képernyőn, hogy a változások megjelenjenek.
            // Ebben az esetben a drawString közvetlenül a tft-re ír, ami nem ideális,
            // jobb lenne egy UI Label komponenst frissíteni.
        }
    }

    void handleButton2Event(const UIButton::ButtonEvent &event) {
        DEBUG("FMScreen: Button 2 event! ID: %d, Label: '%s', State: %d\n", event.id, event.label.c_str(), (int)event.state);
        if (event.state == UIButton::ButtonState::Pressed) { /* Gomb 2 akció */ }
    }

    void handleButton3Event(const UIButton::ButtonEvent &event) {
        DEBUG("FMScreen: Button 3 event! ID: %d, Label: '%s', State: %d\n", event.id, event.label.c_str(), (int)event.state);
        if (event.state == UIButton::ButtonState::Pressed) { /* Gomb 3 akció */ }
    }

  private:
    // UI komponensek létrehozása és elhelyezése
    void layoutComponents() {
        //const int16_t screenWidth = tft.width();
        const int16_t screenHeight = tft.height();
        const int16_t buttonHeight = UIButton::DEFAULT_BUTTON_HEIGHT; // Gombok magassága
        const int16_t gap = 3;                                        // Rés a gombok között
        const int16_t margin = 5;                                     // Szegély a képernyő szélétől

        // Teljes használható szélesség a gomboknak és réseknek, figyelembe véve a margókat
        // const int16_t totalUsableWidth = screenWidth - (2 * margin);

        // Egy gomb szélessége
        // 3 gomb + 2 rés = totalUsableWidth
        // const int16_t buttonWidth = (totalUsableWidth - (2 * gap)) / 3;
        const int16_t buttonWidth = UIButton::DEFAULT_BUTTON_WIDTH; // Alapértelmezett gomb szélesség

        // Gombok Y pozíciója (pl. a képernyő aljához közel)
        const int16_t buttonY = screenHeight - buttonHeight - margin;

        // Első gomb X pozíciója
        int16_t currentX = margin;

        // Gombok ID-jának beállítása (opcionális, de hasznos lehet az eseménykezelőben)
        const uint8_t BUTTON1_ID = 1;
        const uint8_t BUTTON2_ID = 2;
        const uint8_t BUTTON3_ID = 3;

        button1 = std::make_shared<UIButton>(tft, BUTTON1_ID, Rect(currentX, buttonY), "Gomb1");
        button1->setEventCallback([this](const UIButton::ButtonEvent &event) { this->handleButton1Event(event); });
        addChild(button1);

        currentX += buttonWidth + gap;
        button2 = std::make_shared<UIButton>(tft, BUTTON2_ID, Rect(currentX, buttonY), "Gomb2");
        button2->setEventCallback([this](const UIButton::ButtonEvent &event) { this->handleButton2Event(event); });
        addChild(button2);

        currentX += buttonWidth + gap;
        Rect button3Bounds(currentX, buttonY, buttonWidth, buttonHeight);
        button3 = std::make_shared<UIButton>(tft, BUTTON3_ID, button3Bounds, "Gomb3");
        button3->setEventCallback([this](const UIButton::ButtonEvent &event) { this->handleButton3Event(event); });
        addChild(button3);
    }
};

#endif // __FM_SCREEN_H