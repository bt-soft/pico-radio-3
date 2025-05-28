#ifndef __SCREEN_MANAGER_H
#define __SCREEN_MANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <vector>

#include "defines.h"

// Forward deklarációk
class UIComponent;

// Touch esemény struktúra
struct TouchEvent {
    uint16_t x, y;
    bool pressed;
    uint32_t timestamp;

    TouchEvent(uint16_t x, uint16_t y, bool pressed) : x(x), y(y), pressed(pressed), timestamp(millis()) {}
};

// Alap komponens interface
class Component {
  public:
    virtual ~Component() = default;

    // Touch esemény kezelése - visszatérés: true ha feldolgozta az eseményt
    virtual bool handleTouch(const TouchEvent &event) = 0;

    // Loop hívás - ezt minden komponens megkapja
    virtual void loop() = 0;

    // Rajzolás
    virtual void draw() = 0;

    // Komponens aktiválása/deaktiválása
    virtual void setActive(bool active) { isActive = active; }
    virtual bool getActive() const { return isActive; }

    // Láthatóság
    virtual void setVisible(bool visible) { isVisible = visible; }
    virtual bool getVisible() const { return isVisible; }

  protected:
    bool isActive = true;
    bool isVisible = true;
};

// Kompozit komponens (gyerekkomponenseket tartalmazhat)
class CompositeComponent : public Component {
  protected:
    std::vector<std::shared_ptr<Component>> children;

  public:
    virtual ~CompositeComponent() = default;

    // Gyerek komponens hozzáadása
    void addChild(std::shared_ptr<Component> child) { children.push_back(child); }

    // Gyerek komponens eltávolítása
    void removeChild(std::shared_ptr<Component> child) {
        children.erase(std::remove(children.begin(), children.end(), child), children.end());
    } // Touch esemény továbbítása gyerekeknek (csak ha nem kezeltük le)
    virtual bool handleTouch(const TouchEvent &event) override {
        if (!isActive)
            return false;

        DEBUG("CompositeComponent handleTouch: (%d,%d) pressed=%s children=%d\n", event.x, event.y, event.pressed ? "true" : "false", children.size());

        // Először magunk próbáljuk kezelni
        if (handleOwnTouch(event)) {
            DEBUG("CompositeComponent: handled by self\n");
            return true; // Feldolgoztuk, nem adjuk tovább
        }

        // Ha nem kezeltük, továbbítjuk a gyerekeknek (visszafelé, hogy a felső rétegek elsőbbséget kapjanak)
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
            if ((*it)->getActive() && (*it)->handleTouch(event)) {
                DEBUG("CompositeComponent: handled by child\n");
                return true; // Egy gyerek feldolgozta
            }
        }

        DEBUG("CompositeComponent: not handled\n");
        return false; // Senki sem kezelte
    }

    // Loop hívás továbbítása minden gyereknek
    virtual void loop() override {
        if (!isActive)
            return;

        // Először saját loop logika
        handleOwnLoop();

        // Majd minden gyerek megkapja (megszakítás nélkül)
        for (auto &child : children) {
            if (child->getActive()) {
                child->loop();
            }
        }
    } // Rajzolás továbbítása
    virtual void draw() override {
        if (!isVisible)
            return;

        // Először saját rajzolás
        drawSelf();

        // Majd gyerekek rajzolása
        for (auto &child : children) {
            if (child->getVisible()) {
                child->draw();
            }
        }
    } // Ellenőrzi, hogy bármely gyerek újrarajzolást igényel-e
    virtual bool needsChildRedraw() const {
        // Egyszerűsítjük: ha bármely gyerek létezik és látható, újrarajzolást kérünk
        // Ez biztosítja, hogy minden látható komponens frissüljön
        for (const auto &child : children) {
            if (child->getVisible()) {
                return true; // Konzervatív megközelítés: mindig újrarajzolunk ha van látható gyerek
            }
        }
        return false;
    }

  protected:
    // Ezeket a leszármazott osztályok implementálják
    virtual bool handleOwnTouch(const TouchEvent &event) { return false; }
    virtual void handleOwnLoop() {}
    virtual void drawSelf() {}
};

// Képernyő alap osztály
class Screen : public CompositeComponent {
  protected:
    TFT_eSPI &tft;
    String name;
    bool needsRedraw = true;
    class ScreenManager *screenManager = nullptr;

  public:
    Screen(TFT_eSPI &tft, const String &name) : tft(tft), name(name) {}
    virtual ~Screen() = default;

    const String &getName() const { return name; }

    // ScreenManager beállítása
    void setScreenManager(class ScreenManager *manager) { screenManager = manager; }

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
    } // Paraméterek átadása a képernyőnek
    virtual void setParameters(void *params) { onParametersSet(params); } // Újrarajzolás szükségességének ellenőrzése
    virtual bool isRedrawNeeded() const {
        if (needsRedraw) {
            return true;
        }
        // Ellenőrizzük a gyerek komponenseket is
        return CompositeComponent::needsChildRedraw();
    }

    // Újrarajzolás jelölése
    void markForRedraw() { needsRedraw = true; }

    // Rajzolás felülírása hogy csak szükség esetén rajzoljon
    virtual void draw() override {
        if (!isVisible || !isRedrawNeeded()) {
            return;
        }

        // Ha szükséges az újrarajzolás, töröljük a képernyőt
        if (needsRedraw) {
            tft.fillScreen(TFT_BLACK); // vagy bármilyen háttérszín
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

// Forward deklarációk a képernyőkhöz
class MainScreen;
class MenuScreen;
class InfoScreen;
class VolumeScreen;

// Képernyő factory típus
using ScreenFactory = std::function<std::shared_ptr<Screen>(TFT_eSPI &)>;

// Képernyőkezelő
class ScreenManager {
  private:
    TFT_eSPI &tft;
    std::map<String, ScreenFactory> screenFactories;
    std::shared_ptr<Screen> currentScreen;
    String previousScreenName;

  public:
    ScreenManager(TFT_eSPI &tft) : tft(tft) {
        // Beépített screen factory-k regisztrálása
        registerDefaultScreenFactories();
    }

    // Képernyő factory regisztrálása
    void registerScreenFactory(const String &screenName, ScreenFactory factory) { screenFactories[screenName] = factory; }

    // Képernyő váltás név alapján
    bool switchToScreen(const String &screenName, void *params = nullptr) {
        // Ha már ez a képernyő aktív, nem csinálunk semmit
        if (currentScreen && currentScreen->getName() == screenName) {
            return true;
        }

        // Factory keresése
        auto it = screenFactories.find(screenName);
        if (it == screenFactories.end()) {
            DEBUG("ScreenManager: Screen factory not found for '%s'\n", screenName.c_str());
            return false;
        }

        // Jelenlegi képernyő törlése
        if (currentScreen) {
            previousScreenName = currentScreen->getName();
            currentScreen->deactivate();
            currentScreen.reset(); // Memória felszabadítása
            DEBUG("ScreenManager: Destroyed screen '%s'\n", previousScreenName.c_str());
        } // Új képernyő létrehozása
        currentScreen = it->second(tft);
        if (currentScreen) {
            currentScreen->setScreenManager(this);
            if (params) {
                currentScreen->setParameters(params);
            }
            currentScreen->activate();
            DEBUG("ScreenManager: Created and activated screen '%s'\n", screenName.c_str());
            return true;
        } else {
            DEBUG("ScreenManager: Failed to create screen '%s'\n", screenName.c_str());
        }

        return false;
    } // Vissza az előző képernyőre
    bool goBack() {
        if (!previousScreenName.isEmpty()) {
            return switchToScreen(previousScreenName);
        }
        return false;
    }

    // Touch esemény kezelése
    bool handleTouch(const TouchEvent &event) {
        if (currentScreen && currentScreen->getActive()) {
            return currentScreen->handleTouch(event);
        }
        return false;
    }

    // Loop hívás
    void loop() {
        if (currentScreen && currentScreen->getActive()) {
            currentScreen->loop();
        }
    }

    // Rajzolás
    void draw() {
        if (currentScreen && currentScreen->getVisible()) {
            // Csak akkor rajzolunk, ha valóban szükséges
            if (currentScreen->isRedrawNeeded()) {
                currentScreen->draw();
            }
        }
    }

    // Aktuális képernyő lekérdezése
    std::shared_ptr<Screen> getCurrentScreen() const { return currentScreen; }

    // Előző képernyő neve
    String getPreviousScreenName() const { return previousScreenName; }

  private:
    // Beépített screen factory-k regisztrálása
    void registerDefaultScreenFactories();
};

#endif // __SCREEN_MANAGER_H
