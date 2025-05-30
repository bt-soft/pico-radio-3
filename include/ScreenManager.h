#ifndef __SCREEN_MANAGER_H
#define __SCREEN_MANAGER_H

#include <TFT_eSPI.h>
#include <functional>

#include "uicomponents/IScreenManager.h"
#include "uicomponents/UIScreen.h"

// Deferred action struktúra - biztonságos képernyőváltáshoz
struct DeferredAction {
    enum Type { SwitchScreen, GoBack };

    Type type;
    String screenName;
    void *params;

    DeferredAction(Type t, const String &name = "", void *p = nullptr) : type(t), screenName(name), params(p) {}
};

// Képernyő factory típus
using ScreenFactory = std::function<std::shared_ptr<UIScreen>(TFT_eSPI &)>;

// Képernyőkezelő
class ScreenManager : public IScreenManager {

  private:
    TFT_eSPI &tft;
    std::map<String, ScreenFactory> screenFactories;
    std::shared_ptr<UIScreen> currentScreen;
    String previousScreenName;

    // Deferred action queue - biztonságos képernyőváltáshoz
    std::queue<DeferredAction> deferredActions;
    bool processingEvents = false;

  public:
    ScreenManager(TFT_eSPI &tft) : tft(tft) {
        // Beépített screen factory-k regisztrálása
        registerDefaultScreenFactories();
    }

    // Képernyő factory regisztrálása
    void registerScreenFactory(const String &screenName, ScreenFactory factory) { screenFactories[screenName] = factory; }

    // Deferred képernyő váltás - biztonságos váltás eseménykezelés közben
    void deferSwitchToScreen(const String &screenName, void *params = nullptr) {
        DEBUG("ScreenManager: Deferring switch to screen '%s'\n", screenName.c_str());
        deferredActions.push(DeferredAction(DeferredAction::SwitchScreen, screenName, params));
    }

    // Deferred vissza váltás
    void deferGoBack() {
        DEBUG("ScreenManager: Deferring go back\n");
        deferredActions.push(DeferredAction(DeferredAction::GoBack));
    }

    // Deferred actions feldolgozása - a main loop-ban hívandó
    void processDeferredActions() {
        while (!deferredActions.empty()) {
            const DeferredAction &action = deferredActions.front();

            DEBUG("ScreenManager: Processing deferred action type=%d\n", (int)action.type);

            if (action.type == DeferredAction::SwitchScreen) {
                immediateSwitch(action.screenName, action.params);
            } else if (action.type == DeferredAction::GoBack) {
                immediateGoBack();
            }

            deferredActions.pop();
        }
    }

    // Képernyő váltás név alapján - biztonságos verzió - IScreenManager
    bool switchToScreen(const String &screenName, void *params = nullptr) override {
        if (processingEvents) {
            // Eseménykezelés közben - halasztott váltás
            deferSwitchToScreen(screenName, params);
            return true;
        } else {
            // Biztonságos - azonnali váltás
            return immediateSwitch(screenName, params);
        }
    }

    // Azonnali képernyő váltás - csak biztonságos kontextusban hívható
    bool immediateSwitch(const String &screenName, void *params = nullptr) {
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
        }

        // Új képernyő létrehozása
        currentScreen = it->second(tft);
        if (currentScreen) {
            currentScreen->setManager(this);
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
    }

    // Vissza az előző képernyőre - biztonságos verzió - IScreenManager
    bool goBack() override {
        if (processingEvents) {
            // Eseménykezelés közben - halasztott váltás
            deferGoBack();
            return true;
        } else {
            // Biztonságos - azonnali váltás
            return immediateGoBack();
        }
    }

    // Azonnali visszaváltás - csak biztonságos kontextusban hívható
    bool immediateGoBack() {
        if (!previousScreenName.isEmpty()) {
            return immediateSwitch(previousScreenName);
        }
        return false;
    }

    // Touch esemény kezelése
    bool handleTouch(const TouchEvent &event) {
        if (currentScreen && currentScreen->getActive()) {
            processingEvents = true;
            bool result = currentScreen->handleTouch(event);
            processingEvents = false;
            return result;
        }
        return false;
    }

    // Rotary encoder esemény kezelése
    bool handleRotary(const RotaryEvent &event) {
        if (currentScreen && currentScreen->getActive()) {
            processingEvents = true;
            bool result = currentScreen->handleRotary(event);
            processingEvents = false;
            return result;
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
    std::shared_ptr<UIScreen> getCurrentScreen() const { return currentScreen; }

    // Előző képernyő neve
    String getPreviousScreenName() const { return previousScreenName; }

  private:
    // Beépített screen factory-k regisztrálása
    void registerDefaultScreenFactories();
};

#endif // __SCREEN_MANAGER_H