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

// Gomb komponens
class Button : public UIComponent {
  public:
    // Gomb típusok
    enum class ButtonType {
        Pushable,  // Egyszerű nyomógomb
        Toggleable // Váltógomb (on/off)
    };

    // Gomb állapotok
    enum class ButtonState {
        Off = 0,
        On,
        Disabled,
        CurrentActive, // Jelenleg aktív mód jelzése
        Pressed,       // Nyomva van
        LongPressed    // Hosszú nyomás
    };

    // Gomb esemény struktúra
    struct ButtonEvent {
        uint8_t id;
        String label;
        ButtonState state;
        uint32_t timestamp;

        ButtonEvent(uint8_t id, const String &label, ButtonState state) : id(id), label(label), state(state), timestamp(millis()) {}
    };

  private:
    uint8_t buttonId;
    String text;
    ButtonType buttonType = ButtonType::Pushable;
    ButtonState buttonState = ButtonState::Off;
    uint8_t textSize = 2;
    uint8_t cornerRadius = 5;
    bool useMiniFont = false;
    uint32_t longPressThreshold = 1000; // ms
    uint32_t pressStartTime = 0;

    std::function<void(const ButtonEvent &)> eventCallback;
    std::function<void()> clickCallback; // Backward compatibility

    // Gomb állapot színek
    struct StateColors {
        uint16_t background;
        uint16_t border;
        uint16_t text;
        uint16_t led;
    };

    StateColors getStateColors() const {
        StateColors colors;

        if (buttonState == ButtonState::Disabled) {
            colors.background = TFT_DARKGREY;
            colors.border = TFT_DARKGREY;
            colors.text = TFT_LIGHTGREY;
            colors.led = TFT_BLACK;
        } else if (pressed || buttonState == ButtonState::Pressed) {
            // Pressed állapot - gradiens effekt színek
            colors.background = darkenColor(getBaseBackgroundColor(), 6);
            colors.border = TFT_ORANGE;
            colors.text = TFT_WHITE;
            colors.led = TFT_ORANGE;
        } else {
            colors.background = getBaseBackgroundColor();
            colors.text = TFT_WHITE;
            colors.led = getLedColor();

            // Border színe állapot szerint
            switch (buttonState) {
            case ButtonState::On:
                colors.border = TFT_GREEN;
                break;
            case ButtonState::CurrentActive:
                colors.border = TFT_BLUE;
                break;
            default:
                colors.border = TFT_WHITE;
                break;
            }
        }

        return colors;
    }

    uint16_t getBaseBackgroundColor() const {
        return TFT_COLOR(65, 65, 114); // Kék-szürke alap
    }

    uint16_t getLedColor() const {
        if (buttonState == ButtonState::On) {
            return TFT_GREEN;
        } else if (buttonType == ButtonType::Pushable && pressed) {
            return TFT_ORANGE;
        } else if (buttonType == ButtonType::Toggleable && buttonState == ButtonState::Off) {
            return TFT_COLOR(10, 128, 30); // Sötétzöld
        }
        return TFT_BLACK; // Nincs LED
    }

    // Szín sötétítése gradiens effekthez
    uint16_t darkenColor(uint16_t color, uint8_t steps) const {
        uint8_t r = (color & 0xF800) >> 11;
        uint8_t g = (color & 0x07E0) >> 5;
        uint8_t b = (color & 0x001F);

        uint8_t darkenAmount = steps > 0 ? (steps * 4) : 0;

        r = (r > darkenAmount) ? r - darkenAmount : 0;
        g = (g > darkenAmount) ? g - darkenAmount : 0;
        b = (b > darkenAmount) ? b - darkenAmount : 0;

        return (r << 11) | (g << 5) | b;
    }

    // Gradiens effekt rajzolása pressed állapotban
    void drawPressedEffect(const StateColors &colors) {
        const uint8_t steps = 6;
        uint8_t stepWidth = bounds.width / steps;
        uint8_t stepHeight = bounds.height / steps;

        for (uint8_t i = 0; i < steps; i++) {
            uint16_t fadedColor = darkenColor(getBaseBackgroundColor(), i * 5);
            tft.fillRoundRect(bounds.x + i * stepWidth / 2, bounds.y + i * stepHeight / 2, bounds.width - i * stepWidth, bounds.height - i * stepHeight, cornerRadius, fadedColor);
        }
    }

  public:
    Button(TFT_eSPI &tft, uint8_t id, const Rect &bounds, const String &text, ButtonType type = ButtonType::Pushable, const ColorScheme &colors = ColorScheme::defaultScheme())
        : UIComponent(tft, bounds, colors), buttonId(id), text(text), buttonType(type) {}

    // Backward compatibility constructor
    Button(TFT_eSPI &tft, const Rect &bounds, const String &text, const ColorScheme &colors = ColorScheme::defaultScheme())
        : UIComponent(tft, bounds, colors), buttonId(0), text(text), buttonType(ButtonType::Pushable) {}

    // Getters & Setters
    uint8_t getId() const { return buttonId; }
    void setId(uint8_t id) { buttonId = id; }

    ButtonType getButtonType() const { return buttonType; }
    void setButtonType(ButtonType type) {
        if (buttonType != type) {
            buttonType = type;
            markForRedraw();
        }
    }

    ButtonState getButtonState() const { return buttonState; }
    void setButtonState(ButtonState state) {
        if (buttonState != state) {
            buttonState = state;
            markForRedraw();
        }
    }

    // Szöveg beállítása
    void setText(const String &newText) {
        if (text != newText) {
            text = newText;
            markForRedraw();
        }
    }
    const String &getText() const { return text; }

    // Szöveg méret
    void setTextSize(uint8_t size) {
        if (textSize != size) {
            textSize = size;
            markForRedraw();
        }
    }
    uint8_t getTextSize() const { return textSize; }

    // Sarok lekerekítés
    void setCornerRadius(uint8_t radius) {
        if (cornerRadius != radius) {
            cornerRadius = radius;
            markForRedraw();
        }
    }
    uint8_t getCornerRadius() const { return cornerRadius; }

    // Mini font használata
    void setUseMiniFont(bool mini) {
        if (useMiniFont != mini) {
            useMiniFont = mini;
            markForRedraw();
        }
    }
    bool getUseMiniFont() const { return useMiniFont; }

    // Hosszú nyomás küszöb
    void setLongPressThreshold(uint32_t threshold) { longPressThreshold = threshold; }
    uint32_t getLongPressThreshold() const { return longPressThreshold; }

    // Event callback beállítása
    void setEventCallback(std::function<void(const ButtonEvent &)> callback) { eventCallback = callback; }

    // Kattintás callback (backward compatibility)
    void setClickCallback(std::function<void()> callback) { clickCallback = callback; }

    virtual void draw() override {
        if (!isVisible || !needsRedraw)
            return;

        StateColors stateColors = getStateColors();

        // Pressed effekt vagy normál háttér
        if (pressed || buttonState == ButtonState::Pressed) {
            drawPressedEffect(stateColors);
        } else {
            tft.fillRoundRect(bounds.x, bounds.y, bounds.width, bounds.height, cornerRadius, stateColors.background);
        }

        // Keret rajzolása
        tft.drawRoundRect(bounds.x, bounds.y, bounds.width, bounds.height, cornerRadius, stateColors.border);

        // Szöveg rajzolása
        if (!text.isEmpty()) {
            tft.setTextSize(useMiniFont ? 1 : textSize);
            tft.setTextColor(stateColors.text);
            tft.setTextDatum(MC_DATUM); // Middle Center

            // Szöveg pozíció finomhangolása
            int16_t textY = bounds.centerY();
            if (useMiniFont)
                textY += 1; // Mini font esetén kicsit lejjebb

            tft.drawString(text, bounds.centerX(), textY);
        }

        // LED csík rajzolása (ha nem mini font és van LED szín)
        if (!useMiniFont && stateColors.led != TFT_BLACK) {
            const uint8_t LED_HEIGHT = 5;
            const uint8_t LED_MARGIN = 10;
            tft.fillRect(bounds.x + LED_MARGIN, bounds.y + bounds.height - LED_HEIGHT - 3, bounds.width - 2 * LED_MARGIN, LED_HEIGHT, stateColors.led);
        }

        needsRedraw = false;
    }

    virtual bool handleTouch(const TouchEvent &event) override {
        if (!isActive || !enabled || buttonState == ButtonState::Disabled)
            return false;
        bool inside = isPointInside(event.x, event.y);

        if (event.pressed && inside && !pressed) {
            // Gomb lenyomás kezdete
            pressed = true;
            pressStartTime = millis();
            markForRedraw();
            return true;

        } else if (event.pressed && pressed) {
            // Gomb nyomva tartás - hosszú nyomás ellenőrzése
            if (millis() - pressStartTime >= longPressThreshold && buttonState != ButtonState::LongPressed) {
                buttonState = ButtonState::LongPressed;
                if (eventCallback) {
                    eventCallback(ButtonEvent(buttonId, text, ButtonState::LongPressed));
                }
                markForRedraw();
            }
            return true;

        } else if (!event.pressed && pressed) {
            // Gomb felengedése
            pressed = false;

            if (inside) {
                if (buttonState == ButtonState::LongPressed) {
                    // Hosszú nyomás után felengedés - nem váltunk állapotot
                    buttonState = ButtonState::On; // Vagy maradjon ahol volt
                } else {
                    // Normál kattintás kezelése
                    if (buttonType == ButtonType::Toggleable) {
                        buttonState = (buttonState == ButtonState::Off) ? ButtonState::On : ButtonState::Off;
                        if (eventCallback) {
                            eventCallback(ButtonEvent(buttonId, text, buttonState));
                        }
                    } else {
                        // Pushable gomb
                        if (eventCallback) {
                            eventCallback(ButtonEvent(buttonId, text, ButtonState::Pressed));
                        }
                        if (clickCallback) {
                            clickCallback();
                        }
                    }
                }
            }

            pressStartTime = 0;
            markForRedraw();
            return true;

        } else if (event.pressed && !inside && pressed) {
            // Ujj lehúzása a gombról - cancel
            pressed = false;
            pressStartTime = 0;
            markForRedraw();
            return true;
        }

        return false;
    }

  protected:
    virtual void onClick(const TouchEvent &event) override {
        // Ez már a handleTouch-ban van kezelve
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
