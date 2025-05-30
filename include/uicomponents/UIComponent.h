#ifndef __UI_COMPONENT_H
#define __UI_COMPONENT_H

#include <TFT_eSPI.h>

#include "Component.h"
#include "defines.h"

// Színsémák
struct ColorScheme {
    uint16_t background;
    uint16_t foreground;
    uint16_t border;
    uint16_t pressedBackground;
    uint16_t pressedForeground;
    uint16_t disabledBackground;
    uint16_t disabledForeground;

    static ColorScheme defaultScheme() {
        return {
            TFT_DARKGREY,  // background
            TFT_WHITE,     // foreground
            TFT_LIGHTGREY, // border
            TFT_BLUE,      // pressedBackground
            TFT_WHITE,     // pressedForeground
            TFT_BLACK,     // disabledBackground
            TFT_DARKGREY   // disabledForeground
        };
    }
};

// Alap UI komponens
class UIComponent : public Component {
  protected:
    TFT_eSPI &tft;
    Rect bounds;
    ColorScheme colors;
    bool enabled = true;
    bool pressed = false;
    bool needsRedraw = true; // Dirty flag

  public:
    UIComponent(TFT_eSPI &tft, const Rect &bounds, const ColorScheme &colors = ColorScheme::defaultScheme()) : tft(tft), bounds(bounds), colors(colors) {}

    virtual ~UIComponent() = default;

    // Pozíció és méret
    void setBounds(const Rect &newBounds) {
        if (bounds.x != newBounds.x || bounds.y != newBounds.y || bounds.width != newBounds.width || bounds.height != newBounds.height) {
            bounds = newBounds;
            markForRedraw();
        }
    }
    const Rect &getBounds() const { return bounds; }

    // Engedélyezés/tiltás
    void setEnabled(bool enable) {
        if (enabled != enable) {
            enabled = enable;
            markForRedraw();
        }
    }
    bool isEnabled() const { return enabled; }

    // Színséma
    void setColorScheme(const ColorScheme &newColors) {
        colors = newColors;
        markForRedraw();
    }
    const ColorScheme &getColorScheme() const { return colors; }

    // Újrarajzolás jelölése
    void markForRedraw() { needsRedraw = true; }
    bool isRedrawNeeded() const { return needsRedraw; }

    virtual void setVisible(bool visible) override {
        Component::setVisible(visible);
        if (visible)
            markForRedraw();
    }

    // Touch területének ellenőrzése
    virtual bool isPointInside(int16_t x, int16_t y) const { return bounds.contains(x, y); }
    virtual bool handleTouch(const TouchEvent &event) override {
        if (!isActive || !enabled)
            return false;

        bool inside = isPointInside(event.x, event.y);
        bool oldPressed = pressed;

        DEBUG("UIComponent handleTouch: (%d,%d) pressed=%s inside=%s bounds=(%d,%d,%d,%d)\n", event.x, event.y, event.pressed ? "true" : "false", inside ? "true" : "false",
              bounds.x, bounds.y, bounds.width, bounds.height);

        if (event.pressed && inside) {
            pressed = true;
            onTouchDown(event);
            // Ha pressed állapot változott, újrarajzolás szükséges
            if (oldPressed != pressed) {
                markForRedraw();
            }
            return true;
        } else if (!event.pressed && pressed) {
            pressed = false;
            if (inside) {
                onTouchUp(event);
                onClick(event);
            } else {
                onTouchCancel(event);
            }
            // Ha pressed állapot változott, újrarajzolás szükséges
            if (oldPressed != pressed) {
                markForRedraw();
            }
            return true;
        }

        return false;
    }

    virtual void loop() override {
        // Alapértelmezetten nincs loop logika
    }

  protected:
    // Eseménykezelő metódusok
    virtual void onTouchDown(const TouchEvent &event) {
        // Debug kimenet
        DEBUG("Touch DOWN at (%d,%d)\n", event.x, event.y);
    }
    virtual void onTouchUp(const TouchEvent &event) {
        // Debug kimenet
        DEBUG("Touch UP at (%d,%d)\n", event.x, event.y);
    }
    virtual void onTouchCancel(const TouchEvent &event) {}
    virtual void onClick(const TouchEvent &event) {
        // Debug kimenet
        DEBUG("CLICK at (%d,%d)\n", event.x, event.y);
    }
};

#endif // __UI_COMPONENT_H