#ifndef _COMPONENT_H
#define _COMPONENT_H

#include <Arduino.h>
#include <TFT_eSPI.h>

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

// Téglalap struktúra
struct Rect {
    int16_t x, y;
    uint16_t width, height;

    Rect(int16_t x = 0, int16_t y = 0, uint16_t width = 0, uint16_t height = 0) : x(x), y(y), width(width), height(height) {}

    bool contains(int16_t px, int16_t py) const { return px >= x && px < x + width && py >= y && py < y + height; }

    int16_t centerX() const { return x + width / 2; }
    int16_t centerY() const { return y + height / 2; }
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

#endif // _COMPONENT_H