#ifndef __UI_DIALOG_SYSTEM_TEST_H
#define __UI_DIALOG_SYSTEM_TEST_H

#include "UIDialogScreen.h"
#include "UIConfirmDialog.h"
#include "ui/ScreenManager.h"
#include <TFT_eSPI.h>

/**
 * Complete dialog system functionality test
 * This class provides comprehensive testing of the UIDialog system
 */
class UIDialogSystemTest : public IUIDialogParent {
private:
    ScreenManager* screenManager;
    TFT_eSPI* tft;
    UIDialogManager* dialogManager;
    int testCounter = 0;
    
public:
    UIDialogSystemTest(ScreenManager* sm, TFT_eSPI* display) 
        : screenManager(sm), tft(display) {
        dialogManager = new UIDialogManager(screenManager, *tft);
        DEBUG("UIDialogSystemTest: Dialog system test initialized\n");
    }
    
    virtual ~UIDialogSystemTest() {
        delete dialogManager;
    }
    
    // IUIDialogParent implementation
    virtual void setDialogResponse(const UIDialogResponse& response) override {
        DEBUG("UIDialogSystemTest: Dialog response received\n");
        DEBUG("  Type: %d, Accepted: %d, ButtonIndex: %d\n", 
              (int)response.dialogType, response.accepted, response.buttonIndex);
        DEBUG("  Value: %s, Label: %s\n", 
              response.value.c_str(), response.label.c_str());
              
        // Continue with next test based on current test counter
        testCounter++;
        runNextTest();
    }
    
    /**
     * Start the comprehensive dialog test sequence
     */
    void startTest() {
        DEBUG("UIDialogSystemTest: Starting dialog system tests\n");
        testCounter = 0;
        runNextTest();
    }
    
private:
    void runNextTest() {
        switch(testCounter) {
            case 0:
                testMessageDialog();
                break;
            case 1:
                testConfirmDialog();
                break;
            case 2:
                testMultiChoiceDialog();
                break;
            case 3:
                testComplexDialog();
                break;
            default:
                DEBUG("UIDialogSystemTest: All tests completed successfully!\n");
                break;
        }
    }
    
    void testMessageDialog() {
        DEBUG("UIDialogSystemTest: Testing Message Dialog\n");
        auto dialog = std::make_shared<UIMessageDialog>(*tft, this, 
            "Test Message", 
            "This is a test message dialog.\nClick OK to continue to next test.");
        dialogManager->showDialog(dialog);
    }
    
    void testConfirmDialog() {
        DEBUG("UIDialogSystemTest: Testing Confirm Dialog\n");
        auto dialog = std::make_shared<UIConfirmDialog>(*tft, this, 
            "Test Confirmation", 
            "Do you want to continue with the test?\nThis tests OK/Cancel buttons.");
        dialogManager->showDialog(dialog);
    }
    
    void testMultiChoiceDialog() {
        DEBUG("UIDialogSystemTest: Testing Multi-Choice Dialog\n");
        std::vector<String> choices = {
            "Option A - First choice",
            "Option B - Second choice", 
            "Option C - Third choice",
            "Option D - Fourth choice"
        };
        auto dialog = std::make_shared<UIMultiChoiceDialog>(*tft, this, 
            "Test Multi-Choice", 
            "Select one of the following options:", 
            choices);
        dialogManager->showDialog(dialog);
    }
    
    void testComplexDialog() {
        DEBUG("UIDialogSystemTest: Testing Complex Dialog\n");
        std::vector<String> complexChoices = {
            "Save and Exit",
            "Exit without Saving", 
            "Cancel",
            "Save as New File",
            "Reset to Defaults"
        };
        auto dialog = std::make_shared<UIMultiChoiceDialog>(*tft, this, 
            "Complex Test Dialog", 
            "This dialog has many options to test\nscrolling and layout capabilities.\nWhat would you like to do?", 
            complexChoices);
        dialogManager->showDialog(dialog);
    }
    
public:
    /**
     * Quick integration test - shows a simple dialog
     */
    void quickTest() {
        DEBUG("UIDialogSystemTest: Running quick integration test\n");
        auto dialog = std::make_shared<UIMessageDialog>(*tft, this, 
            "Quick Test", 
            "Dialog system integration test.\nIf you see this, the system works!");
        dialogManager->showDialog(dialog);
    }
    
    /**
     * Get the dialog manager instance (for integration with other systems)
     */
    UIDialogManager* getDialogManager() { return dialogManager; }
};

/**
 * Standalone function to quickly test dialog system functionality
 * Call this from main.cpp for immediate testing
 */
inline void testUIDialogSystem(ScreenManager& screenManager, TFT_eSPI& tft) {
    DEBUG("Starting standalone UI Dialog System test\n");
    
    static UIDialogSystemTest* tester = nullptr;
    if (!tester) {
        tester = new UIDialogSystemTest(&screenManager, &tft);
    }
    
    tester->quickTest();
}

#endif // __UI_DIALOG_SYSTEM_TEST_H
