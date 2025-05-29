#ifndef __UI_DIALOG_BASE_H
#define __UI_DIALOG_BASE_H

#include "IUIDialogParent.h"
#include "UIButton.h"
#include "UIComponents.h"
#include "UIScreen.h"
#include <functional>

// Dialog constants - similar to DialogBase
#define UI_DLG_BACKGROUND_COLOR TFT_DARKGREY
#define UI_DLG_OVERLAY_COLOR TFT_COLOR(90, 90, 90)

#define UI_DLG_Y_POS_OFFSET 20        // Dialog starts this much higher from center
#define UI_DLG_HEADER_H 30            // Header height
#define UI_DLG_CLOSE_BTN_SIZE 20      // Close button size
#define UI_DLG_CLOSE_BUTTON_ID 254    // Close button ID
#define UI_DLG_CLOSE_BUTTON_LABEL "X" // Close button label

// Standard dialog button IDs
#define UI_DLG_OK_BUTTON_ID 1                          // OK button ID
#define UI_DLG_CANCEL_BUTTON_ID UI_DLG_CLOSE_BUTTON_ID // Cancel button ID (same as close)

// Multi dialog button ID start
#define UI_DLG_MULTI_BTN_ID_START 10 // Multi button starting ID

#define UI_DLG_BTN_H 30                                  // Button height in dialog
#define UI_DLG_BUTTON_Y_GAP 10                           // Button gap from dialog bottom
#define UI_DLG_BTN_GAP 10                                // Gap between buttons
#define UI_DIALOG_DEFAULT_BUTTON_TEXT_PADDING_X (2 * 15) // Button text padding

/**
 * Base class for UI dialogs - adapted from DialogBase for the UI framework
 * This class inherits from UIScreen to integrate with the ScreenManager system
 */
class UIDialogBase : public UIScreen {
  private:
    String title;                        // Dialog title
    String message;                      // Dialog message
    uint16_t messageY;                   // Message Y coordinate
    uint16_t closeButtonX, closeButtonY; // Close button position
  protected:
    IUIDialogParent *pParent;                    // Parent object that created the dialog
    uint16_t dialogX, dialogY, dialogW, dialogH; // Dialog dimensions
    uint16_t contentY;                           // Y position where content should start
    int buttonCount = 0;                         // Track number of buttons added

    // UI Components
    std::shared_ptr<Panel> overlayPanel;   // Full-screen overlay
    std::shared_ptr<Panel> dialogPanel;    // Dialog container
    std::shared_ptr<Panel> headerPanel;    // Dialog header
    std::shared_ptr<Panel> contentPanel;   // Dialog content area
    std::shared_ptr<Panel> buttonPanel;    // Dialog buttons area
    std::shared_ptr<Label> titleLabel;     // Title label
    std::shared_ptr<Label> messageLabel;   // Message label
    std::shared_ptr<UIButton> closeButton; // Close button

  public:
    /**
     * Constructor
     * @param pParent Parent screen/object
     * @param tft TFT object
     * @param w Dialog width
     * @param h Dialog height
     * @param title Dialog title
     * @param message Dialog message (optional)
     */
    UIDialogBase(IUIDialogParent *pParent, TFT_eSPI &tft, uint16_t w, uint16_t h, const String &title, const String &message = "")
        : UIScreen(tft, "Dialog"), pParent(pParent), dialogW(w), dialogH(h), title(title), message(message) {

        // Calculate dialog position (centered on screen)
        dialogX = (tft.width() - w) / 2;
        dialogY = ((tft.height() - h) / 2) - UI_DLG_Y_POS_OFFSET; // Slightly higher

        messageY = dialogY + (title.length() > 0 ? UI_DLG_HEADER_H + 15 : 5);
        contentY = messageY + (message.length() > 0 ? 15 : 0);

        createDialogComponents();
    }

    virtual ~UIDialogBase() = default; /**
                                        * Get dialog title
                                        */
    const String &getTitle() const { return title; }

    /**
     * Set parent for dialog response handling
     */
    void setParent(IUIDialogParent *parent) { pParent = parent; }

    /**
     * Set dialog response and notify parent
     */
    virtual void setDialogResponse(const UIDialogResponse &response) {
        if (pParent) {
            pParent->setDialogResponse(response);
        }
    }

    /**
     * Close dialog with specified button response
     */
    virtual void closeDialog(uint8_t buttonId = UI_DLG_CLOSE_BUTTON_ID, const String &label = UI_DLG_CLOSE_BUTTON_LABEL,
                             UIButton::ButtonState state = UIButton::ButtonState::Pressed) {
        setDialogResponse(UIDialogResponse(buttonId, label, state));

        // Close dialog by switching back to previous screen
        if (iScreenManager) {
            iScreenManager->goBack();
        }
    }

    // Override touch handling to implement close button
    virtual bool handleTouch(const TouchEvent &event) override {
        // First let the UI components handle the touch
        if (UIScreen::handleTouch(event)) {
            return true;
        }

        // Handle close button manually (for the X in header)
        if (event.pressed) {
            if (event.x >= closeButtonX && event.x <= closeButtonX + UI_DLG_CLOSE_BTN_SIZE && event.y >= closeButtonY && event.y <= closeButtonY + UI_DLG_CLOSE_BTN_SIZE) {
                closeDialog();
                return true;
            }
        }

        return false;
    }

    // Override rotary handling - by default ESC/back closes dialog
    virtual bool handleRotary(const RotaryEvent &event) override {
        // First let the UI components handle the rotary
        if (UIScreen::handleRotary(event)) {
            return true;
        }

        // Double-click to close dialog (common pattern)
        if (event.buttonState == RotaryEvent::ButtonState::DoubleClicked) {
            closeDialog();
            return true;
        }

        return false;
    }

  protected:
    /**
     * Create dialog UI components
     */
    virtual void createDialogComponents() {
        // Color schemes
        ColorScheme overlayColors = ColorScheme::defaultScheme();
        overlayColors.background = UI_DLG_OVERLAY_COLOR;
        overlayColors.border = TFT_TRANSPARENT;

        ColorScheme dialogColors = ColorScheme::defaultScheme();
        dialogColors.background = UI_DLG_BACKGROUND_COLOR;
        dialogColors.border = TFT_WHITE;

        ColorScheme headerColors = ColorScheme::defaultScheme();
        headerColors.background = TFT_NAVY;
        headerColors.foreground = TFT_WHITE;
        headerColors.border = TFT_BLUE;

        ColorScheme contentColors = ColorScheme::defaultScheme();
        contentColors.background = TFT_TRANSPARENT;
        contentColors.border = TFT_TRANSPARENT;

        ColorScheme labelColors = ColorScheme::defaultScheme();
        labelColors.background = TFT_TRANSPARENT;
        labelColors.foreground = TFT_WHITE;

        ColorScheme closeButtonColors = ColorScheme::defaultScheme();
        closeButtonColors.background = TFT_TRANSPARENT;
        closeButtonColors.foreground = TFT_WHITE;
        closeButtonColors.pressedBackground = TFT_RED;

        // Create overlay panel (full screen with semi-transparent background)
        overlayPanel = std::make_shared<Panel>(tft, Rect(0, 0, tft.width(), tft.height()), overlayColors);
        overlayPanel->setDrawBackground(true);

        // Create dialog panel
        dialogPanel = std::make_shared<Panel>(tft, Rect(dialogX, dialogY, dialogW, dialogH), dialogColors);
        dialogPanel->setDrawBackground(true);

        // Create header panel (if title exists)
        if (title.length() > 0) {
            headerPanel = std::make_shared<Panel>(tft, Rect(0, 0, dialogW, UI_DLG_HEADER_H), headerColors);
            headerPanel->setDrawBackground(true);

            // Title label
            titleLabel = std::make_shared<Label>(tft, Rect(10, 5, dialogW - 40, UI_DLG_HEADER_H - 10), title, labelColors);
            titleLabel->setTextSize(1);
            titleLabel->setTextDatum(ML_DATUM);

            // Close button in header
            closeButtonX = dialogX + dialogW - UI_DLG_CLOSE_BTN_SIZE - 5;
            closeButtonY = dialogY + 5;

            closeButton = std::make_shared<UIButton>(tft, UI_DLG_CLOSE_BUTTON_ID, Rect(dialogW - UI_DLG_CLOSE_BTN_SIZE - 5, 5, UI_DLG_CLOSE_BTN_SIZE, UI_DLG_CLOSE_BTN_SIZE),
                                                     UI_DLG_CLOSE_BUTTON_LABEL, UIButton::ButtonType::Pushable, closeButtonColors);
            closeButton->setEventCallback([this](const UIButton::ButtonEvent &event) {
                if (event.state == UIButton::ButtonState::Pressed) {
                    closeDialog();
                }
            });

            headerPanel->addChild(titleLabel);
            headerPanel->addChild(closeButton);
            dialogPanel->addChild(headerPanel);
        }

        // Create content panel
        uint16_t contentStartY = title.length() > 0 ? UI_DLG_HEADER_H : 0;
        uint16_t contentHeight = dialogH - contentStartY - UI_DLG_BTN_H - UI_DLG_BUTTON_Y_GAP;

        contentPanel = std::make_shared<Panel>(tft, Rect(0, contentStartY, dialogW, contentHeight), contentColors);

        // Message label (if message exists)
        if (message.length() > 0) {
            messageLabel = std::make_shared<Label>(tft, Rect(10, 10, dialogW - 20, 20), message, labelColors);
            messageLabel->setTextSize(1);
            messageLabel->setTextDatum(ML_DATUM);
            contentPanel->addChild(messageLabel);
        }

        // Button panel
        buttonPanel = std::make_shared<Panel>(tft, Rect(0, dialogH - UI_DLG_BTN_H - UI_DLG_BUTTON_Y_GAP, dialogW, UI_DLG_BTN_H + UI_DLG_BUTTON_Y_GAP), contentColors);

        dialogPanel->addChild(contentPanel);
        dialogPanel->addChild(buttonPanel);

        // Add panels to screen
        addChild(overlayPanel);
        overlayPanel->addChild(dialogPanel);
    }

    /**
     * Draw overlay with dotted pattern (similar to DialogBase)
     */
    virtual void drawOverlay() {
        // Draw semi-transparent overlay using dotted pattern
        uint16_t color = UI_DLG_OVERLAY_COLOR;
        for (uint32_t y = 0; y < tft.height(); y += 2) {
            for (uint32_t x = 0; x < tft.width(); x += 2) {
                tft.drawPixel(x, y, color);
            }
        }
    }

    /**
     * Override draw to ensure proper overlay rendering
     */
    virtual void drawSelf() override {
        if (needsRedraw) {
            // Draw overlay first
            drawOverlay();
            needsRedraw = false;
        }
    }

    /**
     * Get content panel for derived classes to add custom content
     */
    std::shared_ptr<Panel> getContentPanel() { return contentPanel; }

    /**
     * Get button panel for derived classes to add custom buttons
     */
    std::shared_ptr<Panel> getButtonPanel() { return buttonPanel; }

    /**
     * Add a button to the dialog
     */
    void addDialogButton(uint8_t id, const String &text, std::function<void()> callback = nullptr) {
        if (!buttonPanel)
            return;

        ColorScheme buttonColors = ColorScheme::defaultScheme();
        buttonColors.background = TFT_DARKGREY;
        buttonColors.foreground = TFT_WHITE;
        buttonColors.pressedBackground = TFT_BLUE; // Calculate button position (simple horizontal layout)
        int currentButtonCount = buttonCount;
        int buttonWidth = 80;
        int startX = (dialogW - buttonWidth) / 2; // Center single button

        if (currentButtonCount > 0) {
            // Multiple buttons - adjust layout
            int totalWidth = (currentButtonCount + 1) * buttonWidth + currentButtonCount * UI_DLG_BTN_GAP;
            startX = (dialogW - totalWidth) / 2;
            startX += currentButtonCount * (buttonWidth + UI_DLG_BTN_GAP);
        }

        auto button = std::make_shared<UIButton>(tft, id, Rect(startX, 5, buttonWidth, UI_DLG_BTN_H), text, UIButton::ButtonType::Pushable, buttonColors);

        button->setEventCallback([this, id, text, callback](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::ButtonState::Pressed) {
                if (callback) {
                    callback();
                }
                setDialogResponse(UIDialogResponse(id, text, event.state));

                // Auto-close dialog unless it's a special button
                if (id != UI_DLG_CLOSE_BUTTON_ID) {
                    closeDialog(id, text, event.state);
                }
            }
        });
        buttonPanel->addChild(button);
        buttonCount++; // Increment button counter
    }
};

#endif // __UI_DIALOG_BASE_H
