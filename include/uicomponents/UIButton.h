#ifndef __UI_BUTTON_H
#define __UI_BUTTON_H

#include "UIComponent.h"
#include <functional>

class UIButton : public UIComponent {

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
            // Pressed állapot - TftButton.h alapján
            colors.background = getBaseBackgroundColor(); // A gradiens effekt fogja kezelni a sötétítést
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
        // LED csak toggleable gomboknál
        if (buttonType != ButtonType::Toggleable) {
            return TFT_BLACK; // Pushable gomboknál nincs LED
        }

        if (buttonState == ButtonState::On) {
            return TFT_GREEN;
        } else if (buttonState == ButtonState::Off) {
            return TFT_COLOR(10, 128, 30); // Sötétzöld
        }
        return TFT_BLACK; // Egyéb állapotokban nincs LED
    }

    // Szín sötétítése gradiens effekthez - TftButton.h alapján
    uint16_t darkenColor(uint16_t color, uint8_t amount) const {
        // Kivonjuk a piros, zöld és kék színösszetevőket
        uint8_t r = (color & 0xF800) >> 11;
        uint8_t g = (color & 0x07E0) >> 5;
        uint8_t b = (color & 0x001F);

        // Finomítjuk a csökkentési mértéket, figyelembe véve a színösszetevők közötti eltéréseket
        uint8_t darkenAmount = amount > 0 ? (amount >> 3) : 0;

        // A csökkentésnél biztosítjuk, hogy ne menjenek 0 alá az értékek
        r = (r > darkenAmount) ? r - darkenAmount : 0;
        g = (g > darkenAmount) ? g - darkenAmount : 0;
        b = (b > darkenAmount) ? b - darkenAmount : 0;

        // Visszaalakítjuk a színt 16 bites RGB formátumba
        return (r << 11) | (g << 5) | b;
    }

    // Gradiens effekt rajzolása pressed állapotban - TftButton.h alapján
    void drawPressedEffect() {
        const uint8_t steps = 6; // TFT_BUTTON_DARKEN_COLORS_STEPS
        uint8_t stepWidth = bounds.width / steps;
        uint8_t stepHeight = bounds.height / steps;

        // Az oldState színét használjuk alapként, mint a TftButton.h-ban
        uint16_t baseColor = getBaseBackgroundColor();

        for (uint8_t i = 0; i < steps; i++) {
            uint16_t fadedColor = darkenColor(baseColor, i * 30); // Erősebb sötétítés, mint TftButton.h-ban
            tft.fillRoundRect(bounds.x + i * stepWidth / 2, bounds.y + i * stepHeight / 2, bounds.width - i * stepWidth, bounds.height - i * stepHeight, cornerRadius, fadedColor);
        }
    }

  public:
    UIButton(TFT_eSPI &tft, uint8_t id, const Rect &bounds, const String &text, ButtonType type = ButtonType::Pushable, const ColorScheme &colors = ColorScheme::defaultScheme())
        : UIComponent(tft, bounds, colors), buttonId(id), text(text), buttonType(type) {}

    // Backward compatibility constructor
    UIButton(TFT_eSPI &tft, const Rect &bounds, const String &text, const ColorScheme &colors = ColorScheme::defaultScheme())
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

        StateColors stateColors = getStateColors(); // Pressed effekt vagy normál háttér - TftButton.h logika alapján
        if (pressed || buttonState == ButtonState::Pressed) {
            drawPressedEffect();
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

        // LED csík rajzolása (csak toggleable gomboknál, ha nem mini font és van LED szín)
        if (buttonType == ButtonType::Toggleable && !useMiniFont && stateColors.led != TFT_BLACK) {
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

#endif // __UI_BUTTON_H