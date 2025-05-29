#ifndef __COMPONENT_H
#define __COMPONENT_H

#include "defines.h"
#include <Arduino.h>

// Touch esemény struktúra
struct TouchEvent {
    uint16_t x, y;
    bool pressed;
    uint32_t timestamp;

    TouchEvent(uint16_t x, uint16_t y, bool pressed) : x(x), y(y), pressed(pressed), timestamp(millis()) {}
};

// Rotary encoder esemény struktúra
struct RotaryEvent {
    enum Direction { None, Up, Down };
    enum ButtonState { NotPressed, Clicked, DoubleClicked };

    Direction direction;
    ButtonState buttonState;
    uint32_t timestamp;

    RotaryEvent(Direction dir, ButtonState btnState) : direction(dir), buttonState(btnState), timestamp(millis()) {}
};

// Deferred action struktúra - biztonságos képernyőváltáshoz
struct DeferredAction {
    enum Type { SwitchScreen, GoBack };

    Type type;
    String screenName;
    void *params;

    DeferredAction(Type t, const String &name = "", void *p = nullptr) : type(t), screenName(name), params(p) {}
};

// Alap komponens interface
class Component {
  public:
    virtual ~Component() = default;

    // Touch esemény kezelése - visszatérés: true ha feldolgozta az eseményt
    virtual bool handleTouch(const TouchEvent &event) = 0;

    // Rotary encoder esemény kezelése - visszatérés: true ha feldolgozta az eseményt
    virtual bool handleRotary(const RotaryEvent &event) { return false; }

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

#endif // __COMPONENT_H