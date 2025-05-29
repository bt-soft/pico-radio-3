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
    } /**
       * Close dialog with specified button response
       */
    virtual void closeDialog(uint8_t buttonId = UI_DLG_CLOSE_BUTTON_ID, const String &label = UI_DLG_CLOSE_BUTTON_LABEL,
                             UIButton::ButtonState state = UIButton::ButtonState::Pressed) {
        DEBUG("Closing dialog with buttonId=%d, label=%s\n", buttonId, label.c_str());

        // Create response for close/cancel
        UIDialogResponse response;
        response.accepted = false; // Close/cancel is always false
        response.buttonIndex = buttonId;
        response.value = label;
        response.dialogType = UIDialogType::Confirm;

        setDialogResponse(response);

        // Close dialog by switching back to previous screen
        if (iScreenManager) {
            DEBUG("Going back to previous screen\n");
            iScreenManager->goBack();
        } else {
            DEBUG("ERROR: No ScreenManager available!\n");
        }
    } // Override touch handling to implement close button and modal behavior
    virtual bool handleTouch(const TouchEvent &event) override {
        DEBUG("UIDialogBase handleTouch: x=%d, y=%d, pressed=%d\n", event.x, event.y, event.pressed);

        // Handle close button manually (for the X in header)
        if (event.pressed && title.length() > 0) {
            if (event.x >= closeButtonX && event.x <= closeButtonX + UI_DLG_CLOSE_BTN_SIZE && event.y >= closeButtonY && event.y <= closeButtonY + UI_DLG_CLOSE_BTN_SIZE) {
                DEBUG("Close button clicked\n");
                closeDialog();
                return true;
            }
        }

        // Check if touch is within dialog bounds - if not, ignore (modal behavior)
        if (event.x < dialogX || event.x > dialogX + dialogW || event.y < dialogY || event.y > dialogY + dialogH) {
            DEBUG("Touch outside dialog bounds - ignoring\n");
            return true; // Consume event to maintain modal behavior
        }

        // Let the UI components handle the touch if it's within dialog
        return UIScreen::handleTouch(event);
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

    /**
     * Override draw to ensure proper overlay rendering
     */
    virtual void draw() override {
        // First draw overlay background
        drawOverlay();

        // Then draw close button manually in header (if title exists)
        if (title.length() > 0) {
            // Draw close button
            tft.fillRect(closeButtonX, closeButtonY, UI_DLG_CLOSE_BTN_SIZE, UI_DLG_CLOSE_BTN_SIZE, TFT_RED);
            tft.drawRect(closeButtonX, closeButtonY, UI_DLG_CLOSE_BTN_SIZE, UI_DLG_CLOSE_BTN_SIZE, TFT_WHITE);

            // Draw X
            tft.setTextColor(TFT_WHITE);
            tft.setTextSize(1);
            tft.setTextDatum(MC_DATUM);
            tft.drawString("X", closeButtonX + UI_DLG_CLOSE_BTN_SIZE / 2, closeButtonY + UI_DLG_CLOSE_BTN_SIZE / 2);
        }

        // Finally draw all UI components
        UIScreen::draw();
    }

  protected: /**
              * Create dialog UI components
              */
    virtual void createDialogComponents() {
        // Clear screen first and draw overlay
        tft.fillScreen(TFT_BLACK);
        drawOverlay();

        // Color schemes
        ColorScheme dialogColors = ColorScheme::defaultScheme();
        dialogColors.background = UI_DLG_BACKGROUND_COLOR;
        dialogColors.border = TFT_WHITE;

        ColorScheme headerColors = ColorScheme::defaultScheme();
        headerColors.background = TFT_NAVY;
        headerColors.foreground = TFT_WHITE;

        ColorScheme contentColors = ColorScheme::defaultScheme();
        contentColors.background = UI_DLG_BACKGROUND_COLOR;
        contentColors.foreground = TFT_WHITE;

        ColorScheme labelColors = ColorScheme::defaultScheme();
        labelColors.background = TFT_TRANSPARENT;
        labelColors.foreground = TFT_WHITE;

        // Create main dialog panel at calculated absolute position
        dialogPanel = std::make_shared<Panel>(tft, Rect(dialogX, dialogY, dialogW, dialogH), dialogColors);
        dialogPanel->setDrawBackground(true);

        uint16_t currentY = 0; // Now relative to dialog panel

        // Create header panel (if title exists)
        if (title.length() > 0) {
            headerPanel = std::make_shared<Panel>(tft, Rect(0, currentY, dialogW, UI_DLG_HEADER_H), headerColors);
            headerPanel->setDrawBackground(true);

            // Title label - positioned within header (relative to header)
            titleLabel = std::make_shared<Label>(tft, Rect(8, 6, dialogW - 50, 18), title, labelColors);
            titleLabel->setTextSize(1);
            titleLabel->setTextDatum(ML_DATUM);

            // Close button coordinates (absolute screen coordinates for manual drawing)
            closeButtonX = dialogX + dialogW - UI_DLG_CLOSE_BTN_SIZE - 5;
            closeButtonY = dialogY + 5;

            headerPanel->addChild(titleLabel);
            dialogPanel->addChild(headerPanel);
            currentY += UI_DLG_HEADER_H;
        }

        // Content area - calculate proper height
        uint16_t buttonAreaHeight = UI_DLG_BTN_H + UI_DLG_BUTTON_Y_GAP + 10;
        uint16_t contentHeight = dialogH - currentY - buttonAreaHeight;
        contentPanel = std::make_shared<Panel>(tft, Rect(0, currentY, dialogW, contentHeight), contentColors);
        contentPanel->setDrawBackground(true);

        // Message label (if message exists) - properly positioned in content area
        if (message.length() > 0) {
            messageLabel = std::make_shared<Label>(tft, Rect(10, 10, dialogW - 20, contentHeight - 20), message, labelColors);
            messageLabel->setTextSize(1);
            messageLabel->setTextDatum(TL_DATUM);
            contentPanel->addChild(messageLabel);
        }

        dialogPanel->addChild(contentPanel);

        // Button panel - positioned at bottom of dialog (relative to dialog panel)
        uint16_t buttonPanelY = dialogH - buttonAreaHeight;
        buttonPanel = std::make_shared<Panel>(tft, Rect(0, buttonPanelY, dialogW, buttonAreaHeight), contentColors);
        buttonPanel->setDrawBackground(false); // Transparent background
        dialogPanel->addChild(buttonPanel);

        // Add main dialog panel to screen
        addChild(dialogPanel);
    }

    /**
     * Draw overlay with dotted pattern (similar to DialogBase)
     */
    virtual void drawOverlay() {
        // Draw semi-transparent overlay using dotted pattern
        uint16_t color = UI_DLG_OVERLAY_COLOR;
        for (uint32_t y = 0; y < tft.height(); y += 3) {
            for (uint32_t x = 0; x < tft.width(); x += 3) {
                tft.drawPixel(x, y, color);
            }
        }
    } /**
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
        buttonColors.pressedBackground = TFT_BLUE;
        buttonColors.border = TFT_LIGHTGREY;

        // Calculate button position for horizontal layout (relative to button panel)
        int buttonWidth = 80;
        int totalButtons = buttonCount + 1;
        int totalGaps = totalButtons > 1 ? totalButtons - 1 : 0;
        int totalWidth = totalButtons * buttonWidth + totalGaps * UI_DLG_BTN_GAP;
        int startX = (dialogW - totalWidth) / 2;
        int buttonX = startX + buttonCount * (buttonWidth + UI_DLG_BTN_GAP);

        // Position button within button panel with proper vertical centering
        int buttonY = (UI_DLG_BTN_H + UI_DLG_BUTTON_Y_GAP + 10 - UI_DLG_BTN_H) / 2;

        // Create button positioned in button panel (relative coordinates to button panel)
        auto button = std::make_shared<UIButton>(tft, id, Rect(buttonX, buttonY, buttonWidth, UI_DLG_BTN_H), text, UIButton::ButtonType::Pushable, buttonColors);

        button->setEventCallback([this, id, text, callback](const UIButton::ButtonEvent &event) {
            DEBUG("Dialog button pressed: id=%d, text=%s, state=%d\n", id, text.c_str(), (int)event.state);

            if (event.state == UIButton::ButtonState::Pressed) {
                if (callback) {
                    callback();
                }

                // Create response
                UIDialogResponse response;
                response.accepted = (id == UI_DLG_OK_BUTTON_ID);
                response.buttonIndex = id;
                response.value = text;
                response.dialogType = UIDialogType::Confirm; // Default type

                // Notify parent
                setDialogResponse(response);

                // Close dialog - let the ScreenManager handle it
                if (iScreenManager) {
                    DEBUG("Closing dialog by going back to previous screen\n");
                    iScreenManager->goBack();
                }
            }
        });

        buttonPanel->addChild(button);
        buttonCount++;
    }
};

#endif // __UI_DIALOG_BASE_H
