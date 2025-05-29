#ifndef __UI_DIALOG_TEST_H
#define __UI_DIALOG_TEST_H

#include "ScreenManager.h"
#include "UIConfirmDialog.h"
#include "UIDialogBase.h"
#include "UIDialogScreen.h"

/**
 * Simple test class to validate dialog system functionality
 * This can be used to test dialogs independently of the main screens
 */
class UIDialogTest : public IUIDialogParent {
  private:
    TFT_eSPI &tft;
    ScreenManager *screenManager;
    UIDialogManager *dialogManager;

  public:
    UIDialogTest(TFT_eSPI &tft, ScreenManager *sm) : tft(tft), screenManager(sm) {
        if (sm) {
            dialogManager = new UIDialogManager(sm, tft);
        }
    }

    virtual ~UIDialogTest() {
        if (dialogManager) {
            delete dialogManager;
        }
    }

    // IUIDialogParent implementation
    virtual void setDialogResponse(const UIDialogResponse &response) override {
        DEBUG("UIDialogTest received response: type=%d, accepted=%d, buttonIndex=%d, value='%s'\n", (int)response.dialogType, response.accepted, response.buttonIndex,
              response.value.c_str());

        switch (response.dialogType) {
        case UIDialogType::Confirm:
            if (response.accepted) {
                DEBUG("Confirmation accepted\n");
            } else {
                DEBUG("Confirmation rejected\n");
            }
            break;

        case UIDialogType::Message:
            DEBUG("Message acknowledged\n");
            break;

        case UIDialogType::MultiChoice:
            if (response.accepted && response.buttonIndex >= 0) {
                DEBUG("Choice selected: %d - '%s'\n", response.buttonIndex, response.value.c_str());
            } else {
                DEBUG("Choice cancelled\n");
            }
            break;
        }
    }

    // Test methods
    void testConfirmDialog() {
        DEBUG("Testing confirm dialog...\n");
        if (dialogManager) {
            dialogManager->showConfirmDialog("Test Confirm", "Do you want to continue with the test?", this);
        }
    }

    void testMessageDialog() {
        DEBUG("Testing message dialog...\n");
        if (dialogManager) {
            dialogManager->showMessageDialog("Test Message", "This is a test message dialog.\nIt can display multiple lines of text.", this);
        }
    }

    void testChoiceDialog() {
        DEBUG("Testing choice dialog...\n");
        if (dialogManager) {
            std::vector<String> choices = {"Option A", "Option B", "Option C", "Cancel"};
            dialogManager->showMultiChoiceDialog("Test Choice", choices, "Please select an option:", this);
        }
    }

    // Run all tests
    void runTests() {
        DEBUG("=== Starting Dialog System Tests ===\n");

        // You can uncomment these one by one to test
        // testConfirmDialog();
        // testMessageDialog();
        // testChoiceDialog();

        DEBUG("=== Dialog System Tests Complete ===\n");
    }
};

#endif // __UI_DIALOG_TEST_H
