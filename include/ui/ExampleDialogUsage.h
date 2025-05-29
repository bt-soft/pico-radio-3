#ifndef __EXAMPLE_DIALOG_USAGE_H
#define __EXAMPLE_DIALOG_USAGE_H

#include "ui/UIConfirmDialog.h"
#include "ui/UIDialogScreen.h"

/**
 * Example screen showing how to use the dialog system
 * This demonstrates how any screen can implement IUIDialogParent and show dialogs
 */
class ExampleDialogUsageScreen : public UIScreen, public IUIDialogParent {
  private:
    std::shared_ptr<UIButton> showConfirmButton;
    std::shared_ptr<UIButton> showMessageButton;
    std::shared_ptr<UIButton> showChoiceButton;
    std::shared_ptr<UIButton> backButton;
    std::shared_ptr<Label> statusLabel;

    UIDialogManager *dialogManager = nullptr;

  public:
    ExampleDialogUsageScreen(TFT_eSPI &tft) : UIScreen(tft, "ExampleDialogScreen") { createComponents(); }

    virtual ~ExampleDialogUsageScreen() = default;

    // Set dialog manager (should be called after screen creation)
    void setDialogManager(UIDialogManager *dm) { dialogManager = dm; }

    // IUIDialogParent implementation
    virtual void setDialogResponse(const UIDialogResponse &response) override {
        String responseText = "Response: ID=" + String(response.buttonId) + ", Label=" + response.label + ", State=" + String((int)response.state);

        statusLabel->setText(responseText);

        DEBUG("Dialog response received: %s\n", responseText.c_str());

        // Handle specific responses
        switch (response.buttonId) {
        case UI_DLG_OK_BUTTON_ID:
            DEBUG("User clicked OK\n");
            break;
        case UI_DLG_CANCEL_BUTTON_ID:
            DEBUG("User clicked Cancel\n");
            break;
        case UI_DLG_CLOSE_BUTTON_ID:
            DEBUG("User closed dialog\n");
            break;
        default:
            if (response.buttonId >= UI_DLG_MULTI_BTN_ID_START) {
                int choiceIndex = UIMultiChoiceDialog::getChoiceIndex(response.buttonId);
                DEBUG("User selected choice %d: %s\n", choiceIndex, response.label.c_str());
            }
            break;
        }
    }

  protected:
    void createComponents() {
        // Color schemes
        ColorScheme buttonColors = ColorScheme::defaultScheme();
        buttonColors.background = TFT_DARKGREY;
        buttonColors.foreground = TFT_WHITE;
        buttonColors.pressedBackground = TFT_BLUE;

        ColorScheme backButtonColors = ColorScheme::defaultScheme();
        backButtonColors.background = TFT_RED;
        backButtonColors.foreground = TFT_WHITE;
        backButtonColors.pressedBackground = TFT_MAROON;

        ColorScheme labelColors = ColorScheme::defaultScheme();
        labelColors.background = TFT_TRANSPARENT;
        labelColors.foreground = TFT_WHITE;

        // Create buttons
        showConfirmButton = std::make_shared<UIButton>(tft, 1, Rect(20, 50, 120, 40), "Confirm Dialog", UIButton::ButtonType::Pushable, buttonColors);
        showConfirmButton->setEventCallback([this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::ButtonState::Pressed && dialogManager) {
                dialogManager->showConfirmDialog("Confirm Action", "Are you sure you want to continue?", this);
            }
        });

        showMessageButton = std::make_shared<UIButton>(tft, 2, Rect(20, 100, 120, 40), "Message Dialog", UIButton::ButtonType::Pushable, buttonColors);
        showMessageButton->setEventCallback([this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::ButtonState::Pressed && dialogManager) {
                dialogManager->showMessageDialog("Information", "This is an information message.", this);
            }
        });

        showChoiceButton = std::make_shared<UIButton>(tft, 3, Rect(20, 150, 120, 40), "Choice Dialog", UIButton::ButtonType::Pushable, buttonColors);
        showChoiceButton->setEventCallback([this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::ButtonState::Pressed && dialogManager) {
                std::vector<String> choices = {"Option 1", "Option 2", "Option 3"};
                dialogManager->showMultiChoiceDialog("Select Option", choices, "Choose one:", this);
            }
        });

        backButton = std::make_shared<UIButton>(tft, 4, Rect(20, tft.height() - 60, 100, 40), "Back", UIButton::ButtonType::Pushable, backButtonColors);
        backButton->setEventCallback([this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::ButtonState::Pressed && iScreenManager) {
                iScreenManager->goBack();
            }
        });

        // Status label
        statusLabel = std::make_shared<Label>(tft, Rect(160, 50, tft.width() - 180, 140), "No dialog response yet", labelColors);
        statusLabel->setTextSize(1);
        statusLabel->setTextDatum(TL_DATUM);

        // Add components
        addChild(showConfirmButton);
        addChild(showMessageButton);
        addChild(showChoiceButton);
        addChild(backButton);
        addChild(statusLabel);
    }

    virtual void onActivate() override {
        UIScreen::onActivate();
        statusLabel->setText("Ready to show dialogs");
    }
};

#endif // __EXAMPLE_DIALOG_USAGE_H
