#ifndef __IUI_DIALOG_PARENT_H
#define __IUI_DIALOG_PARENT_H

#include "UIButton.h"
#include <Arduino.h>

// Dialog types
enum class UIDialogType {
    Confirm,    // Confirmation dialog (OK/Cancel)
    Message,    // Message dialog (OK only)
    MultiChoice // Multi-choice dialog (multiple options)
};

// Dialog response structure - equivalent to DialogBase's ButtonTouchEvent
struct UIDialogResponse {
    uint8_t buttonId;
    String label;
    UIButton::ButtonState state;
    UIDialogType dialogType;
    bool accepted;   // true if OK/confirm was pressed, false if cancelled
    int buttonIndex; // Index of selected button (for multi-choice dialogs)
    String value;    // Selected value text (for multi-choice dialogs)

    UIDialogResponse(uint8_t id = 0, const String &lbl = "", UIButton::ButtonState st = UIButton::ButtonState::Off, UIDialogType type = UIDialogType::Message, bool acc = false,
                     int btnIdx = -1, const String &val = "")
        : buttonId(id), label(lbl), state(st), dialogType(type), accepted(acc), buttonIndex(btnIdx), value(val) {}
};

// Interface for dialog parent classes - similar to IDialogParent
class IUIDialogParent {
  public:
    virtual ~IUIDialogParent() = default;

    // Called when dialog generates a response (button click, close, etc.)
    virtual void setDialogResponse(const UIDialogResponse &response) = 0;
};

#endif // __IUI_DIALOG_PARENT_H
