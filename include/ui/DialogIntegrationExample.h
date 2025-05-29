#ifndef __DIALOG_INTEGRATION_EXAMPLE_H
#define __DIALOG_INTEGRATION_EXAMPLE_H

#include "MainScreen.h"
#include "MenuScreen.h"
#include "ui/ScreenManager.h"
#include "ui/UIDialogScreen.h"

/**
 * Complete example showing how to integrate the dialog system
 * with the existing ScreenManager and screens
 */
class DialogIntegrationExample {
  private:
    TFT_eSPI &tft;
    ScreenManager screenManager;
    UIDialogManager *dialogManager;

  public:
    DialogIntegrationExample(TFT_eSPI &tft) : tft(tft), screenManager(tft) { setupDialogSystem(); }

    virtual ~DialogIntegrationExample() {
        if (dialogManager) {
            delete dialogManager;
        }
    }

    void setupDialogSystem() {
        DEBUG("Setting up dialog system integration...\n");

        // 1. Register default screen factories (including dialog screens)
        screenManager.registerDefaultScreenFactories();

        // 2. Create dialog manager
        dialogManager = new UIDialogManager(&screenManager, tft);

        // 3. Set up dialog manager for existing screens that support it
        // Note: This would typically be done when creating screens or
        // when switching to them, but here's the concept:
        setupScreenDialogSupport();

        DEBUG("Dialog system integration complete\n");
    }

    void setupScreenDialogSupport() {
        // This shows how to add dialog support to screens that implement IUIDialogParent
        // In a real application, you'd do this when creating or activating screens

        // Example: When MenuScreen is created, set its dialog manager
        // This could be done in a screen factory or activation callback
        DEBUG("Dialog support configured for compatible screens\n");
    }

    // Helper method to demonstrate dialog manager usage
    void demonstrateDialogUsage() {
        DEBUG("=== Dialog System Demonstration ===\n");

        // Switch to main screen first
        screenManager.switchToScreen("MainScreen");

        // Then switch to menu screen to test dialog functionality
        screenManager.switchToScreen("MenuScreen");

        // Get current screen and check if it supports dialogs
        auto currentScreen = screenManager.getCurrentScreen();
        if (currentScreen) {
            // Try to cast to MenuScreen to set dialog manager
            MenuScreen *menuScreen = dynamic_cast<MenuScreen *>(currentScreen.get());
            if (menuScreen && dialogManager) {
                DEBUG("Setting dialog manager for MenuScreen\n");
                menuScreen->setDialogManager(dialogManager);

                // Now the menu screen can show dialogs when menu items are selected
                DEBUG("MenuScreen now supports dialog functionality\n");
            }
        }

        DEBUG("=== Demonstration Complete ===\n");
    }

    // Main loop for the application
    void loop() { screenManager.loop(); }

    // Draw method
    void draw() { screenManager.draw(); }

    // Event handling
    bool handleTouch(const TouchEvent &event) { return screenManager.handleTouch(event); }

    bool handleRotary(const RotaryEvent &event) { return screenManager.handleRotary(event); }

    // Get screen manager reference for external use
    ScreenManager &getScreenManager() { return screenManager; }

    // Get dialog manager reference
    UIDialogManager *getDialogManager() { return dialogManager; }
};

/**
 * Usage example in main.cpp:
 *
 * #include "DialogIntegrationExample.h"
 *
 * TFT_eSPI tft;
 * DialogIntegrationExample app(tft);
 *
 * void setup() {
 *     // Initialize TFT and other hardware
 *     tft.init();
 *
 *     // Setup dialog integration
 *     app.setupDialogSystem();
 *
 *     // Start with main screen
 *     app.getScreenManager().switchToScreen("MainScreen");
 *
 *     // Demonstrate dialog functionality
 *     app.demonstrateDialogUsage();
 * }
 *
 * void loop() {
 *     // Handle events
 *     TouchEvent touchEvent;
 *     if (getTouchEvent(touchEvent)) {
 *         app.handleTouch(touchEvent);
 *     }
 *
 *     RotaryEvent rotaryEvent;
 *     if (getRotaryEvent(rotaryEvent)) {
 *         app.handleRotary(rotaryEvent);
 *     }
 *
 *     // Update and draw
 *     app.loop();
 *     app.draw();
 * }
 */

#endif // __DIALOG_INTEGRATION_EXAMPLE_H
