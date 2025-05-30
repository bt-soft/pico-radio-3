#ifndef __UI_SCREEN_H
#define __UI_SCREEN_H

#include <TFT_eSPI.h>

#include "CompositeComponent.h"

// Képernyő alap osztály
class UIScreen : public CompositeComponent {
  protected:
    TFT_eSPI &tft;
    String name;
    bool needsRedraw = true;
    class IScreenManager *iMgr = nullptr;

  public:
    UIScreen(TFT_eSPI &tft, const String &name) : tft(tft), name(name) {}
    virtual ~UIScreen() = default;

    // A képernyő nevének elkérése
    const String &getName() const { return name; }

    // IScreenManager beállítása
    void setManager(class IScreenManager *mgr) { iMgr = mgr; }

    // Képernyő aktiválása
    virtual void activate() {
        setActive(true);
        setVisible(true);
        needsRedraw = true;
        onActivate();
    }

    // Képernyő deaktiválása
    virtual void deactivate() {
        onDeactivate();
        setActive(false);
        setVisible(false);
    }

    // Paraméterek átadása a képernyőnek
    virtual void setParameters(void *params) { onParametersSet(params); } // Újrarajzolás szükségességének ellenőrzése
    virtual bool isRedrawNeeded() const {
        if (needsRedraw) {
            return true;
        }
        // Ellenőrizzük a gyerek komponenseket is
        return CompositeComponent::needsChildRedraw();
    }

    // Újrarajzolásra jelölés
    void markForRedraw() { needsRedraw = true; }

    // Rajzolás felülírása hogy csak szükség esetén rajzoljon
    virtual void draw() override {
        if (!isVisible || !isRedrawNeeded()) {
            return;
        }

        // Ha szükséges az újrarajzolás, töröljük a képernyőt
        if (needsRedraw) {
            tft.fillScreen(TFT_COLOR_BACKGROUND);
            needsRedraw = false;
        }

        // Kompozit komponens rajzolása
        CompositeComponent::draw();
    }

  protected:
    // Virtuális metódusok a leszármazott osztályok számára
    virtual void onActivate() {}
    virtual void onDeactivate() {}
    virtual void onParametersSet(void *params) {}
};

#endif // __UI_SCREEN_H
