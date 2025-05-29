#include "ui/ScreenManager.h"
#include "InfoScreen.h"
#include "MainScreen.h"
#include "MenuScreen.h"
#include "VolumeScreen.h"
#include "ui/UIDialogScreen.h"

void ScreenManager::registerDefaultScreenFactories() {
    // MainScreen factory
    registerScreenFactory("MainScreen", [](TFT_eSPI &tft) -> std::shared_ptr<UIScreen> { return std::make_shared<MainScreen>(tft); });

    // MenuScreen factory
    registerScreenFactory("MenuScreen", [](TFT_eSPI &tft) -> std::shared_ptr<UIScreen> { return std::make_shared<MenuScreen>(tft, "Main Menu sanyi"); });

    // InfoScreen factory
    registerScreenFactory("InfoScreen", [](TFT_eSPI &tft) -> std::shared_ptr<UIScreen> { return std::make_shared<InfoScreen>(tft); }); // VolumeScreen factory
    registerScreenFactory("VolumeScreen", [](TFT_eSPI &tft) -> std::shared_ptr<UIScreen> { return std::make_shared<VolumeScreen>(tft); });

    // Note: Dialog screens are created dynamically by UIDialogManager, not through static factories
}
