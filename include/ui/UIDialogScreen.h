#ifndef __UI_DIALOG_SCREEN_H
#define __UI_DIALOG_SCREEN_H

#include "UIConfirmDialog.h"
#include "UIDialogBase.h"

/**
 * Dialog Screen wrapper - allows dialogs to be managed by ScreenManager
 * This class acts as a bridge between the dialog system and screen system
 */
class UIDialogScreen : public UIScreen, public IUIDialogParent {
  private:
    std::shared_ptr<UIDialogBase> dialog;
    IUIDialogParent *originalParent;
    UIDialogResponse lastResponse;
    bool responseReceived = false;

  public:
    UIDialogScreen(TFT_eSPI &tft, std::shared_ptr<UIDialogBase> dlg, IUIDialogParent *parent = nullptr) : UIScreen(tft, "DialogScreen"), dialog(dlg), originalParent(parent) {

        if (dialog) {
            // Replace dialog's parent with this screen
            dialog->setParent(this);

            // Set screen manager reference for dialog
            dialog->setScreenManager(nullptr); // Will be set by ScreenManager when screen is activated
        }
    }

    virtual ~UIDialogScreen() = default;

    // Override onActivate to properly set up dialog
    virtual void onActivate() override {
        UIScreen::onActivate();
        responseReceived = false;

        if (dialog && iScreenManager) {
            dialog->setScreenManager(iScreenManager);
        }
    }

    // Override draw to let dialog handle its own rendering
    virtual void draw() override {
        if (dialog) {
            dialog->draw();
        }
    }

    // Override touch/rotary handling to forward to dialog
    virtual bool handleTouch(const TouchEvent &event) override {
        if (dialog) {
            return dialog->handleTouch(event);
        }
        return false;
    }

    virtual bool handleRotary(const RotaryEvent &event) override {
        if (dialog) {
            return dialog->handleRotary(event);
        }
        return false;
    }

    // IUIDialogParent implementation
    virtual void setDialogResponse(const UIDialogResponse &response) override {
        DEBUG("UIDialogScreen received response: accepted=%d, buttonId=%d, value=%s\n", response.accepted, response.buttonId, response.value.c_str());

        lastResponse = response;
        responseReceived = true;

        // Forward to original parent if exists
        if (originalParent) {
            originalParent->setDialogResponse(response);
        }
    }

    // Get last dialog response
    const UIDialogResponse &getLastResponse() const { return lastResponse; }
    bool hasResponse() const { return responseReceived; }

    // Reset response state
    void resetResponse() { responseReceived = false; }
};

/**
 * Dialog Manager - utility class to show dialogs using ScreenManager
 */
class UIDialogManager {
  private:
    ScreenManager *screenManager;
    TFT_eSPI &tft;

  public:
    UIDialogManager(ScreenManager *sm, TFT_eSPI &tft) : screenManager(sm), tft(tft) {} // Show a confirmation dialog
    void showConfirmDialog(const String &title, const String &message, IUIDialogParent *parent = nullptr) {
        auto dialog = std::make_shared<UIConfirmDialog>(tft, parent, title, message);
        auto dialogScreen = std::make_shared<UIDialogScreen>(tft, dialog, parent);

        // Register the dialog screen temporarily
        screenManager->registerScreenFactory("ConfirmDialog", [dialogScreen](TFT_eSPI &tft) -> std::shared_ptr<UIScreen> { return dialogScreen; });

        // Switch to dialog
        screenManager->switchToScreen("ConfirmDialog");
    } // Show a message dialog
    void showMessageDialog(const String &title, const String &message, IUIDialogParent *parent = nullptr) {
        auto dialog = std::make_shared<UIMessageDialog>(tft, parent, title, message);
        auto dialogScreen = std::make_shared<UIDialogScreen>(tft, dialog, parent);

        // Register the dialog screen temporarily
        screenManager->registerScreenFactory("MessageDialog", [dialogScreen](TFT_eSPI &tft) -> std::shared_ptr<UIScreen> { return dialogScreen; });

        // Switch to dialog
        screenManager->switchToScreen("MessageDialog");
    } // Show a multi-choice dialog
    void showMultiChoiceDialog(const String &title, const std::vector<String> &choices, const String &message = "", IUIDialogParent *parent = nullptr) {
        auto dialog = std::make_shared<UIMultiChoiceDialog>(tft, parent, title, message, choices);
        auto dialogScreen = std::make_shared<UIDialogScreen>(tft, dialog, parent);

        // Register the dialog screen temporarily
        screenManager->registerScreenFactory("MultiChoiceDialog", [dialogScreen](TFT_eSPI &tft) -> std::shared_ptr<UIScreen> { return dialogScreen; });

        // Switch to dialog
        screenManager->switchToScreen("MultiChoiceDialog");
    }
};

#endif // __UI_DIALOG_SCREEN_H
