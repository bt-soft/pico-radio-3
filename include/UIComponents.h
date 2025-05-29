#ifndef __UI_COMPONENTS_H
#define __UI_COMPONENTS_H

#include "ScreenManager.h"
#include "defines.h"
#include <functional>

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

// Téglalap struktúra
struct Rect {
    int16_t x, y;
    uint16_t width, height;

    Rect(int16_t x = 0, int16_t y = 0, uint16_t width = 0, uint16_t height = 0) : x(x), y(y), width(width), height(height) {}

    bool contains(int16_t px, int16_t py) const { return px >= x && px < x + width && py >= y && py < y + height; }

    int16_t centerX() const { return x + width / 2; }
    int16_t centerY() const { return y + height / 2; }
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

// Label komponens (csak szöveg megjelenítése)
class Label : public UIComponent {
  private:
    String text;
    uint8_t textSize = 2;
    uint8_t textDatum = TL_DATUM; // Top Left

  public:
    Label(TFT_eSPI &tft, const Rect &bounds, const String &text, const ColorScheme &colors = ColorScheme::defaultScheme()) : UIComponent(tft, bounds, colors), text(text) {}
    void setText(const String &newText) {
        if (text != newText) {
            text = newText;
            markForRedraw();
        }
    }
    const String &getText() const { return text; }

    void setTextSize(uint8_t size) {
        if (textSize != size) {
            textSize = size;
            markForRedraw();
        }
    }
    uint8_t getTextSize() const { return textSize; }

    void setTextDatum(uint8_t datum) {
        if (textDatum != datum) {
            textDatum = datum;
            markForRedraw();
        }
    }
    uint8_t getTextDatum() const { return textDatum; }

    virtual bool handleTouch(const TouchEvent &event) override {
        // Label nem kezeli a touch eseményeket
        return false;
    }
    virtual void draw() override {
        if (!isVisible || !needsRedraw)
            return;

        uint16_t bgColor = colors.background;
        uint16_t fgColor = enabled ? colors.foreground : colors.disabledForeground;

        // Háttér rajzolása (ha szükséges)
        if (bgColor != TFT_TRANSPARENT) {
            tft.fillRect(bounds.x, bounds.y, bounds.width, bounds.height, bgColor);
        }

        // Szöveg rajzolása
        if (!text.isEmpty()) {
            tft.setTextSize(textSize);
            tft.setTextColor(fgColor);
            tft.setTextDatum(textDatum);

            int16_t textX = bounds.x;
            int16_t textY = bounds.y;

            // Szöveg pozíció beállítása a datum alapján
            switch (textDatum) {
            case MC_DATUM: // Middle Center
                textX = bounds.centerX();
                textY = bounds.centerY();
                break;
            case TC_DATUM: // Top Center
                textX = bounds.centerX();
                break;
            case BC_DATUM: // Bottom Center
                textX = bounds.centerX();
                textY = bounds.y + bounds.height;
                break;
            case ML_DATUM: // Middle Left
                textY = bounds.centerY();
                break;
            case MR_DATUM: // Middle Right
                textX = bounds.x + bounds.width;
                textY = bounds.centerY();
                break;
                // TL_DATUM (Top Left) az alapértelmezett
            }

            tft.drawString(text, textX, textY);
        }

        needsRedraw = false; // Újrarajzolás kész
    }
};

// Panel komponens (más komponenseket tartalmaz)
class Panel : public CompositeComponent {
  protected:
    TFT_eSPI &tft;
    Rect bounds;
    ColorScheme colors;
    bool drawBackground = true;
    bool needsRedraw = true; // Dirty flag hozzáadása

  public:
    Panel(TFT_eSPI &tft, const Rect &bounds, const ColorScheme &colors = ColorScheme::defaultScheme()) : tft(tft), bounds(bounds), colors(colors) {}

    void setBounds(const Rect &newBounds) {
        if (bounds.x != newBounds.x || bounds.y != newBounds.y || bounds.width != newBounds.width || bounds.height != newBounds.height) {
            bounds = newBounds;
            needsRedraw = true;
        }
    }
    const Rect &getBounds() const { return bounds; }

    void setDrawBackground(bool draw) {
        if (drawBackground != draw) {
            drawBackground = draw;
            needsRedraw = true;
        }
    }
    bool getDrawBackground() const { return drawBackground; }

    void setColorScheme(const ColorScheme &newColors) {
        colors = newColors;
        needsRedraw = true;
    }
    const ColorScheme &getColorScheme() const { return colors; } // Újrarajzolás jelölése
    void markForRedraw() { needsRedraw = true; }
    bool isRedrawNeeded() const {
        if (needsRedraw)
            return true;
        // Also check if any child components need redrawing
        return CompositeComponent::needsChildRedraw();
    }
    virtual void setVisible(bool visible) override {
        CompositeComponent::setVisible(visible);
        if (visible)
            needsRedraw = true;
    }

  protected:
    virtual void drawSelf() override {
        if (!isVisible || !needsRedraw)
            return;
        if (drawBackground) {
            // Háttér rajzolása
            tft.fillRect(bounds.x, bounds.y, bounds.width, bounds.height, colors.background);

            // Keret rajzolása
            tft.drawRect(bounds.x, bounds.y, bounds.width, bounds.height, colors.border);
        }

        needsRedraw = false; // Újrarajzolás kész
    }
};

#endif // __UI_COMPONENTS_H
