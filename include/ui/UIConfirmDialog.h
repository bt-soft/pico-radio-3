#ifndef __UI_CONFIRM_DIALOG_H
#define __UI_CONFIRM_DIALOG_H

#include "UIDialogBase.h"

/**
 * Simple confirmation dialog with OK/Cancel buttons
 * Example usage of UIDialogBase
 */
class UIConfirmDialog : public UIDialogBase {
  public:
    UIConfirmDialog(TFT_eSPI &tft, IUIDialogParent *pParent, const String &title, const String &message = "") : UIDialogBase(pParent, tft, 280, 150, title, message) {

        // Add OK and Cancel buttons
        addDialogButton(UI_DLG_OK_BUTTON_ID, "OK");
        addDialogButton(UI_DLG_CANCEL_BUTTON_ID, "Cancel");
    }

    virtual ~UIConfirmDialog() = default;
};

/**
 * Simple message dialog with only OK button
 */
class UIMessageDialog : public UIDialogBase {
  public:
    UIMessageDialog(TFT_eSPI &tft, IUIDialogParent *pParent, const String &title, const String &message = "") : UIDialogBase(pParent, tft, 250, 120, title, message) {

        // Add only OK button
        addDialogButton(UI_DLG_OK_BUTTON_ID, "OK");
    }

    virtual ~UIMessageDialog() = default;
};

/**
 * Custom dialog with multiple options
 */
class UIMultiChoiceDialog : public UIDialogBase {
  private:
    std::vector<String> choices;

  public:
    UIMultiChoiceDialog(TFT_eSPI &tft, IUIDialogParent *pParent, const String &title, const String &message, const std::vector<String> &choices)
        : UIDialogBase(pParent, tft, 300, 200 + (choices.size() * 35), title, message), choices(choices) {

        // Add choice buttons
        for (size_t i = 0; i < choices.size(); i++) {
            addDialogButton(UI_DLG_MULTI_BTN_ID_START + i, choices[i]);
        }

        // Add cancel button
        addDialogButton(UI_DLG_CANCEL_BUTTON_ID, "Cancel");
    }

    virtual ~UIMultiChoiceDialog() = default;

    // Get choice index from button ID
    static int getChoiceIndex(uint8_t buttonId) {
        if (buttonId >= UI_DLG_MULTI_BTN_ID_START) {
            return buttonId - UI_DLG_MULTI_BTN_ID_START;
        }
        return -1;
    }
};

#endif // __UI_CONFIRM_DIALOG_H
