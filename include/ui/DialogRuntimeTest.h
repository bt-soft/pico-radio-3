#ifndef __DIALOG_RUNTIME_TEST_H
#define __DIALOG_RUNTIME_TEST_H

#include "UIDialogScreen.h"
#include "ui/ScreenManager.h"
#include <TFT_eSPI.h>

/**
 * Simple runtime test for dialog system - can be called from main.cpp
 * This provides a quick way to verify dialog functionality works on actual hardware
 */
class DialogRuntimeTest : public IUIDialogParent {
  private:
    UIDialogManager *dialogManager;
    int testStep = 0;

  public:
    DialogRuntimeTest(UIDialogManager *dm) : dialogManager(dm) {}

    // IUIDialogParent implementation
    virtual void setDialogResponse(const UIDialogResponse &response) override {
        DEBUG("DialogRuntimeTest: Response received - Type: %d, Accepted: %d, Value: %s\n", (int)response.dialogType, response.accepted, response.value.c_str());

        // Automatically proceed to next test
        testStep++;
        if (testStep < 3) {
            // Wait a bit and show next dialog
            delay(1000);
            runTest();
        } else {
            DEBUG("DialogRuntimeTest: All tests completed successfully!\n");
        }
    }

    /**
     * Run a quick runtime test of the dialog system
     */
    void runTest() {
        switch (testStep) {
        case 0:
            DEBUG("DialogRuntimeTest: Testing Message Dialog\n");
            dialogManager->showMessageDialog("Runtime Test 1/3", "Message dialog test.\nThis should show an OK button.", this);
            break;

        case 1:
            DEBUG("DialogRuntimeTest: Testing Confirm Dialog\n");
            dialogManager->showConfirmDialog("Runtime Test 2/3", "Confirm dialog test.\nClick OK to continue.", this);
            break;

        case 2:
            DEBUG("DialogRuntimeTest: Testing Multi-Choice Dialog\n");
            {
                std::vector<String> choices = {"Choice A", "Choice B", "Choice C"};
                dialogManager->showMultiChoiceDialog("Runtime Test 3/3", choices, "Multi-choice test.\nSelect any option.", this);
            }
            break;

        default:
            DEBUG("DialogRuntimeTest: Test sequence complete\n");
            break;
        }
    }

    /**
     * Start the test sequence
     */
    void startTest() {
        DEBUG("DialogRuntimeTest: Starting dialog system runtime test\n");
        testStep = 0;
        runTest();
    }
};

/**
 * Convenient function to run a quick dialog test
 * Can be called from main.cpp during setup or on demand
 *
 * Usage in main.cpp:
 * // Add to setup() function after dialog manager initialization:
 * // runDialogRuntimeTest(dialogManager);
 */
inline void runDialogRuntimeTest(UIDialogManager *dialogManager) {
    static DialogRuntimeTest *runtimeTest = nullptr;

    if (!runtimeTest) {
        runtimeTest = new DialogRuntimeTest(dialogManager);
    }

    DEBUG("Starting dialog runtime test...\n");
    runtimeTest->startTest();
}

/**
 * Single quick test - just shows one message dialog
 */
inline void quickDialogTest(UIDialogManager *dialogManager) {
    class QuickTest : public IUIDialogParent {
      public:
        virtual void setDialogResponse(const UIDialogResponse &response) override { DEBUG("Quick dialog test completed - response received\n"); }
    };

    static QuickTest quickTest;

    DEBUG("Running quick dialog test...\n");
    dialogManager->showMessageDialog("Quick Test", "Dialog system is working!\nThis is a quick test message.", &quickTest);
}

#endif // __DIALOG_RUNTIME_TEST_H
