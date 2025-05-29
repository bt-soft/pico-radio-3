#ifndef __COMPOSITE_COMPONENT_H
#define __COMPOSITE_COMPONENT_H

#include "Component.h"
#include <algorithm>
#include <memory>

// Kompozit komponens (gyerekkomponenseket tartalmazhat)
class CompositeComponent : public Component {
  protected:
    std::vector<std::shared_ptr<Component>> children;

  public:
    virtual ~CompositeComponent() = default;

    // Gyerek komponens hozzáadása
    void addChild(std::shared_ptr<Component> child) { children.push_back(child); }

    // Gyerek komponens eltávolítása
    void removeChild(std::shared_ptr<Component> child) { children.erase(std::remove(children.begin(), children.end(), child), children.end()); }

    // Touch esemény továbbítása gyerekeknek (csak ha nem kezeltük le)
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

    // Rotary encoder esemény továbbítása gyerekeknek (csak ha nem kezeltük le)
    virtual bool handleRotary(const RotaryEvent &event) override {
        if (!isActive)
            return false;

        DEBUG("CompositeComponent handleRotary: direction=%d, button=%d, children=%d\n", (int)event.direction, (int)event.buttonState, children.size());

        // Először magunk próbáljuk kezelni
        if (handleOwnRotary(event)) {
            DEBUG("CompositeComponent: rotary handled by self\n");
            return true; // Feldolgoztuk, nem adjuk tovább
        }

        // Ha nem kezeltük, továbbítjuk a gyerekeknek (visszafelé, hogy a felső rétegek elsőbbséget kapjanak)
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
            if ((*it)->getActive() && (*it)->handleRotary(event)) {
                DEBUG("CompositeComponent: rotary handled by child\n");
                return true; // Egy gyerek feldolgozta
            }
        }

        DEBUG("CompositeComponent: rotary not handled\n");
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
    }

    // Rajzolás továbbítása
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
    }

    virtual bool needsChildRedraw() const {
        // Ha bármely gyerek létezik és látható, újrarajzolást kérünk
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
    virtual bool handleOwnRotary(const RotaryEvent &event) { return false; }
    virtual void handleOwnLoop() {}
    virtual void drawSelf() {}
};

#endif // __COMPOSITE_COMPONENT_H
